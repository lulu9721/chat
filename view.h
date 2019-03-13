#ifndef VIEW_H_
#define VIEW_H_
#include<json/json.h>
using namespace std;

//base class
typedef class view
{
    public:
        virtual void process(Json::Value val,int cli_fd) = 0;
        virtual void responce() = 0;
}*Pview;
#endif

