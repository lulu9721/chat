#ifndef _TCPSEVER_H_
#define _TCPSEVER_H_

#include<iostream>
#include<map>
#include<vector>
#include<string>
using namespace std;

class sockPair
{
public:
    sockPair(int first=0,int second=0)
        :_first(first),_second(second){}
    ~sockPair(){}
    void operator=(sockPair &src)
    {
        _first = src._first;
        _second = src._second;  
    }
    int operator[](int index)
    {
        if(index == 0)
        {
            return _first;
        }
        else
            return _second;
        }
private:
    //s[0] for the main to listen pressure
    int _first;
    //s[1] for the worker to listen accepted fd
    int _second;

    friend void main_connected_by_cli(int fd,short event,void *arg);
};

class tcpsever
{
	public:
		tcpsever(const char* ip,unsigned short port,int pth_num);

		void run();
		void create_socket_pair();
		void create_pth();
        
        
	private:
        //init libevent
		struct event_base* _base;
        //main create socket for tcp
		int _main_sock;
        //the number of pthread
		int _pth_num;
        //filled by sockPair
		vector<sockPair> _sockpair_base;
        //the pressure of socket each thread
		map<int,int> _pth_num_map;

        friend void main_connect_by_cli(int fd,short event,void *arg);
};
#endif