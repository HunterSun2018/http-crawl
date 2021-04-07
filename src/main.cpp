#include <iostream>
//#include "http_client.h"
using namespace std;

void run(std::string host, std::string port, std::string target, int version);

int main(int argc, const char* argv[])
{
    run("cn.bing.com", "443", "/?mkt=zh-CN", 11);
    //run("cn.bing.com", "443", "/", 11);

    return 0;
}