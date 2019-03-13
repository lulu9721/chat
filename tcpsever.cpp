#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <pthread.h>
#include <event.h>
#include <string>
#include <error.h>
#include <errno.h>

#include"tcpsever.h"
#include"mpthread.h"
using namespace std;

void main_connect_by_cli(int fd,short event,void *arg);
void recv_thread_pressure(int fd,short event,void* arg);

tcpsever::tcpsever(const char* ip,unsigned short port,int pth_num)
{
	//create server
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if( -1 == fd )
    {
        cerr<<"fd creadt fail;errno:"<<errno<<endl;
        return;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);

    if( -1 == bind(fd,(struct sockaddr*)&saddr,sizeof(saddr)) )
    {
        cerr<<"bind fail;errno:"<<errno<<endl;
        return;
    }

    if( -1 == listen(fd,20) )
    {
        cerr<<"listen fail;errno:"<<errno<<endl;
        return;
    }

	_main_sock = fd;
	_pth_num = pth_num;

	//init libevent
	_base = event_base_new();
}

void tcpsever::run()
{
	//create socketpair for each thread
    create_socket_pair();

	//start thread
    for(int i=0; i<_pth_num; ++i)
    {
        int sock_1 = _sockpair_base[i][1];
        new mpthread(sock_1);
    }

	//add _main_sock to libevent and set callback funtion
    struct event *listen_event  = event_new(_base,_main_sock,EV_READ|EV_PERSIST,main_connect_by_cli,(void*)this);
    if(NULL == listen_event)
    {
        cerr<<"listen event_new fail;errno"<<errno<<endl;
    }
    event_add(listen_event,NULL);

	//loop listen waitfor callback
    event_base_dispatch(_base);
}

void tcpsever::create_socket_pair()
{
	for(int i = 0; i < _pth_num;i++)
	{
        int s[2];
        if( -1 ==socketpair(AF_UNIX,SOCK_STREAM,0,s) )
        {
            cerr<<"sockpair fail;errno:"<<errno<<endl;
        }
        sockPair Spair(s[0],s[1]);

        //push sockpair into _sock_pair_base
		_sockpair_base.push_back(Spair);

        //insert sockPair_0  into map,set num zero
		_pth_num_map.insert(pair<int,int>(s[0],0));

		//set s[0] in libevent(callback when thread send pressure)
		struct event* listen_event = event_new(_base,s[0],EV_READ|EV_PERSIST,recv_thread_pressure,&_pth_num_map);
        if(NULL == listen_event)
        {
            cerr<<"event new fail;errno:"<<errno<<endl; 
            return;
        }

		event_add(listen_event,NULL);
	}
}

//callback function (when main was connected)
void main_connect_by_cli(int fd,short event,void *arg)
{
    tcpsever *mthis = (tcpsever *)arg;
    
    struct sockaddr_in caddr;
    int len = sizeof(caddr);
    //main accept cli then give it to least_pressure_thread
    int cli_fd = accept(fd,(struct sockaddr*)&caddr,(socklen_t*)&len);

    if(cli_fd != -1)
    {
        cout<<"client["<<cli_fd<<"],IP:"<<inet_ntoa(caddr.sin_addr)<<endl;

        map<int,int>::iterator it = mthis->_pth_num_map.begin();
        map<int,int>::iterator it_small = it;
        for(; it != mthis->_pth_num_map.end(); ++it)
        {
            if(it->second < it_small->second)
            {
                it_small = it;
            }
        }
        send(it_small->first,&cli_fd,sizeof(cli_fd),0);
    }
}

//callback function(when threads send their pressure to main)
void recv_thread_pressure(int fd,short event,void* arg)
{

	map<int,int>* P_pth_num_map = (map<int,int>*)arg;

	//recv the pressure from thread
    int num = 0;
    if( 0 < recv(fd,&num,sizeof(num),0))
    {
        cout<<"pthread["<<fd<<"] pressure:"<<num<<endl;
    }

	//renew the map of thread pressure 
	(*P_pth_num_map)[fd] =  num;
}
