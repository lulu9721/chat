#include<iostream>
#include<string.h>
#include<json/json.h>
#include<string>
#include<map>
#include<sys/socket.h>
#include<errno.h>

#include"control.h"
#include"public.h"
#include"view.h"
//#include"view_login.h"
#include"view_register.h"
//#include"view_exit.h"
//#include"view_talk_one.h"
using namespace std;

control::control()
{
    _map.insert( make_pair(MSG_TYPE_REGISTER,new view_register()) );
    //_map.insert( make_pair(MSG_TYPE_LOGIN,new view_login()) );
    //_map.insert( make_pair(MSG_TYPE_EXIT,new view_exit()) );
    //_map.insert( make_pair(MSG_TYPE_TALK_ONE,new view_talk_one()) );
}

void control::process(char *buff,int cli_fd)
{
    //construct json bag
    Json::Value val;
    Json::Reader read;
    if( -1 == read.parse(buff,val) )
    {
        cerr<<"json read fail;errno:"<<errno<<endl;
        return;
    }
    
    //find the aim view
    //use view's process and responce
    if(val["TYPE"].asInt() == 0 || val["TYPE"].asInt() == 1 || val["TYPE"].asInt() == 2 || val["TYPE"].asInt() == 3)
    {
        _map[val["TYPE"].asInt()]->process(val,cli_fd);
        _map[val["TYPE"].asInt()]->responce();
    }

    //if the TYPE was not register with view
    else
    {
        Json::Value val;
        val["reason"] = "error command!";
        send(cli_fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    }
}
