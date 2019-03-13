#ifndef MPTHREAD_H
#define MPTHREAD_H
#include<map>
#include<event.h>
#include"control.h"
//#include"view_exit.h"

using namespace std;

class mpthread
{
	public:
		mpthread(int sockPair_1);
	private:
        //sockPair s[1]
		int _sockPair_1;
        //init libevent
		struct event_base* _base;
        //keep long link with each cli
		map<int,struct event*> _event_map;

        //every thread send the events to control
        control *_control;

        friend void* thread_run(void *arg);
        friend void thread_recv_clifd(int fd,short event,void *arg);
        friend void recv_data_form_cli(int fd,short event,void* arg);
        //friend void view_exit::process(Json::Value val,int cli_fd);
};

#endif 
