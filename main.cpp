#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <errno.h>

#include "tcpsever.h"
using namespace std;

int main(int argc, char const *argv[])
{
    const char* ip = "127.0.0.1";
    unsigned short port = 8000;
    int pth_num = 2;

    //构造服务器对象
    tcpsever ser(ip,port,pth_num);

    //运行服务器
    ser.run();
    return 0;
}
