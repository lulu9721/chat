#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<map>
#include<event.h>
#include<error.h>
#include<errno.h>

#include"mpthread.h"
#include"control.h"

using namespace std;

void* thread_run(void* arg);
void thread_recv_clifd(int fd,short event,void *arg);
void recv_data_form_cli(int fd,short event,void* arg);

mpthread::mpthread(int sockPair_1)
{
    //create its control
    _control = new control;

	_sockPair_1 = sockPair_1;
	
	//start pthread
    pthread_t id;
    pthread_create(&id,NULL,thread_run,(void*)this);
}

//the pthread function
void *thread_run(void *arg)
{
    //the arg is this_point because it need to use its private data
	mpthread* mthis = (mpthread*)arg;

    //init libevent
	mthis->_base = event_base_new();
	
	//callback when main send cli_fd to thread  
    struct event* listen_event = event_new(mthis->_base,mthis->_sockPair_1,EV_READ|EV_PERSIST,thread_recv_clifd,mthis);
    if(NULL == listen_event)
    {
        cerr<<"thread event new fail;errno"<<errno<<endl;
    }

	event_add(listen_event,NULL);

    //loop to listen 
	event_base_dispatch(mthis->_base);

}

//callback function(when main send cli_fd to thread)
void thread_recv_clifd(int fd,short event,void *arg)
{
    mpthread* mthis = (mpthread*)arg;

	//recv cli_fd 
    int cli_fd = 0;
    if( 0 > recv(fd,&cli_fd,sizeof(cli_fd),0) )
    {
        cerr<<"recv cli_fd"<<errno<<endl;
        return;
    }

	//keep a long link with cli
    //set callback function when cli send mssenage
    struct event* listen_event = event_new(mthis->_base,cli_fd,EV_READ|EV_PERSIST,recv_data_form_cli,mthis);
	

	event_add(listen_event,NULL);

	//insert event
    //if cli closed we can delete event from libevent
    mthis->_event_map.insert(pair<int,struct event*>(cli_fd,listen_event));


	//send this thread's pressure to main_thread
    //and the main_thread will select the least pressure thread next time
    int cli_size = mthis->_event_map.size();
    if( -1 == send(fd,&cli_size,sizeof(cli_size),0) )
    {
        cerr<<"send cli_size fail;errno:"<<errno<<endl;
    } 
}

//callback function (when cli send a message)
void recv_data_form_cli(int fd,short event,void* arg)
{
    //cast the argument
    mpthread* mthis = (mpthread*)arg;
	char buff[100] = {0};

    //cli was close without send angthing
    //error exit
    if( 0 >= recv(fd,buff,99,0) )
    {
        cout<<"client ["<<fd<<"] close!"<<endl;
        event_del(mthis->_event_map[fd]);
        mthis->_event_map.erase(fd);
        close(fd);
    }
	
	//send buff to control
    mthis->_control->process(buff,fd);   
}
