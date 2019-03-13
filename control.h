#ifndef CONTROL_H_
#define CONTROL_H_
#include <iostream>
#include <map>
#include "view.h"
#include "public.h"

using namespace std;
class control
{
public:
    control();
    void process(char* buff,int cli_fd);
private:
    //find the view by event type
    map<const int,view*>_map;
};
#endif
