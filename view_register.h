#ifndef _VIEW_REGISTER_
#define _VIEW_REGISTER
#include<iostream>
#include<json/json.h>
#include<mysql/mysql.h>
#include<string>
#include<hiredis/hiredis.h>
#include"view.h"

using namespace std;

class view_register:public view
{
    public:
        bool query_usr_mysql(Json::Value val,MYSQL *mpcon);
        void insert_usr_mysql(Json::Value val,MYSQL *mpcon);
	bool query_usr_redis(Json::Value val,redisContext* c);
	void insert_usr_redis(Json::Value val,redisContext *c);
        void process(Json::Value val,int cli_fd);
        void responce()
        {
            ;
        }
    private:
        string _reason;
        int _cli_fd;
};
#endif

