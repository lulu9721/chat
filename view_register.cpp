#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<json/json.h>
#include<hiredis/hiredis.h>
#include<mysql/mysql.h>
#include<string>
#include<errno.h>
#include"view_register.h"

using namespace std;

void view_register::process(Json::Value val,int cli_fd)
{
    _cli_fd = cli_fd;

    //connect mysql
    MYSQL *mpcon = mysql_init((MYSQL*)0);
     if(!mysql_real_connect(mpcon,"127.0.0.1","root","111111",NULL,3306,NULL,0))
    {
        cerr<<"sql connect fail;errno"<<errno<<endl;
        return;
    }
    //select mysql table
    if(mysql_select_db(mpcon,"socket"))
    {
        cerr<<"select database fail;errno:"<<errno<<endl;
        return;
    }

    //connect redis
    redisContext* c_redis = redisConnect("127.0.0.1",6379);
    if( c_redis->err )
    {
        redisFree(c_redis);
        cerr<<"connect to redisServer fail"<<errno<<endl;
        return;
    }

    if(!query_usr_redis(val,c_redis))
    {
        if(!query_usr_mysql(val,mpcon))
        {
            insert_usr_mysql(val,mpcon);
            Json::Value val;
            val["reason"] = "register sucessfully";
            send(_cli_fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
        }
        else
        {
            insert_usr_redis(val,c_redis);
            Json::Value val;
            val["reason"] = "Usr is exited,please try again!";
            send(_cli_fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
        }
    }
     else
    {
        Json::Value val;
        val["reason"] = "Usr is exited,please try again!";
        send(_cli_fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    }
    redisFree(c_redis); 
}

bool view_register::query_usr_mysql(Json::Value val,MYSQL *mpcon)
{
    MYSQL_RES * mp_res;
    MYSQL_ROW mp_row;

    string name = val["name"].asString();
    name += "';";

    char cmd[128] = "SELECT * FROM Usr WHERE name = '";
    strcat(cmd,name.c_str());

    if(mysql_real_query(mpcon,cmd,strlen(cmd)))
    {
        cerr<<"query name in Usr fail;errno:"<<errno<<endl;
        exit(-1);
    }
    mp_res = mysql_store_result(mpcon);
    if(mp_row = mysql_fetch_row(mp_res))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void view_register::insert_usr_mysql(Json::Value val,MYSQL *mpcon)
{
    string name = val["name"].asString();
    name += "','";
    string password = val["password"].asString();
    password += "');";
    name += password;

    char cmd[128] = "INSERT INTO Usr values('";
    strcat(cmd,name.c_str());

    if(mysql_real_query(mpcon,cmd,strlen(cmd)))
    {
        cerr<<"Insert Usr fail;errno:"<<errno<<endl;
        return;
    }
}

bool view_register::query_usr_redis(Json::Value val,redisContext* c)
{
    string command = "hget usr ";
    string name = val["name"].asString();
    command += name;

    redisReply* r = (redisReply*)redisCommand(c, command.c_str());

    if( NULL == r)  
    {  
        cerr<<"Execut command1 failure:"<<errno<<endl;   
        return 0;  
    }
    if(r->type == REDIS_REPLY_STRING)  
    {  
        freeReplyObject(r);   
        return 1;  
    }
    else if(r->type == REDIS_REPLY_NIL)
    {
        freeReplyObject(r); 
        return 0;
    }
    
}

void view_register::insert_usr_redis(Json::Value val,redisContext *c)
{
    string command = "hset usr ";
    string name = val["name"].asString();
    string password = val["password"].asString();
    command += name;
    command += " ";
    command += password;

    redisReply* r = (redisReply*)redisCommand(c, command.c_str());

    if( NULL == r)  
    {  
        cerr<<"Execut command1 failure:"<<errno<<endl;   
        return;  
    }
    if(!(r->type == REDIS_REPLY_INTEGER))  
    {  
        cerr<<"insert redis error:"<<errno<<endl;  
        freeReplyObject(r);   
        return;  
    }
}
