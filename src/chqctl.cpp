#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t=SOCKET; static constexpr socket_t bad_socket=INVALID_SOCKET;
static void closes(socket_t s){ if(s!=INVALID_SOCKET) closesocket(s); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t=int; static constexpr socket_t bad_socket=-1;
static void closes(socket_t s){ if(s>=0) ::close(s); }
#endif

static bool send_command(std::uint16_t port,const std::string& command,std::string& response){
#ifdef _WIN32
    WSADATA w{}; if(WSAStartup(MAKEWORD(2,2),&w)!=0){response="WSAStartup failed";return false;}
#endif
    socket_t s=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); if(s==bad_socket){response="socket failed";return false;}
    sockaddr_in a{}; a.sin_family=AF_INET; a.sin_port=htons(port); a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if(::connect(s,reinterpret_cast<sockaddr*>(&a),sizeof(a))!=0){ closes(s); response="could not connect to ChaseHQNative on 127.0.0.1:"+std::to_string(port);
#ifdef _WIN32
        WSACleanup();
#endif
        return false; }
    std::string line=command; if(line.empty()||line.back()!='\n') line+='\n';
    if(::send(s,line.data(),static_cast<int>(line.size()),0)<=0){closes(s);response="send failed";return false;}
    response.clear(); char b[4096];
    for(;;){ const int n=::recv(s,b,sizeof(b),0); if(n<=0) break; response.append(b,b+n); }
    closes(s);
#ifdef _WIN32
    WSACleanup();
#endif
    while(!response.empty()&&(response.back()=='\r'||response.back()=='\n')) response.pop_back();
    return true;
}

static bool status_step_remaining_zero(const std::string& s){
    const auto p=s.find("step_remaining="); if(p==std::string::npos) return false;
    auto q=p+15; return q<s.size() && s[q]=='0' && (q+1==s.size() || s[q+1]==' ' || s[q+1]=='\n');
}
static bool wait_for_controlled_run(std::uint16_t port,unsigned timeout_ms=30000){
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(timeout_ms);
    while(std::chrono::steady_clock::now()<deadline){
        std::string r; if(!send_command(port,"status",r)){std::cerr<<"ERR "<<r<<"\n";return false;}
        if(status_step_remaining_zero(r)) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cerr<<"ERR timed out waiting for controlled run to finish\n"; return false;
}
static bool is_run_command(const std::string& line){
    std::istringstream in(line);std::string a,b;in>>a>>b;
    if(a=="run") return true;
    return a=="step" && (b=="frame"||b=="frames");
}
static int execute_one(std::uint16_t port,const std::string& line,bool wait_after,bool echo=true){
    std::string response; if(!send_command(port,line,response)){std::cerr<<"ERR "<<response<<"\n";return 1;}
    if(echo) std::cout<<response<<"\n";
    if(response.rfind("ERR",0)==0) return 1;
    if(wait_after && is_run_command(line) && !wait_for_controlled_run(port)) return 1;
    return 0;
}
static int execute_script(std::uint16_t port,const std::string& path,bool wait_after){
    std::ifstream f(path); if(!f){std::cerr<<"ERR could not open script "<<path<<"\n";return 1;}
    std::string line; unsigned lineno=0;
    while(std::getline(f,line)){++lineno; auto first=line.find_first_not_of(" \t\r\n"); if(first==std::string::npos||line[first]=='#'||line[first]==';')continue;
        std::cout<<"CHQ["<<lineno<<"]> "<<line<<"\n"; if(execute_one(port,line,wait_after,true)) return 1;
    }
    return 0;
}
static void usage(){
    std::cout<<"chqctl v0.66.9.0 - ChaseHQ-Native live Research Workbench client\n"
             <<"Usage: chqctl [--port N] [--wait] COMMAND...\n"
             <<"       chqctl [--port N] shell\n"
             <<"       chqctl [--port N] script FILE [--no-wait]\n\n"
             <<"--wait makes run/step-frame commands synchronous (recommended in pasted command chains).\n"
             <<"Scripts wait after controlled runs by default.\n\n"
             <<"Examples:\n"
             <<"  chqctl status\n  chqctl pause\n  chqctl regs A\n  chqctl reg write A D0 1234\n"
             <<"  chqctl read A 10A096 16\n  chqctl write A 10A096 16 01AA\n"
             <<"  chqctl --wait run 10\n  chqctl watch add A 10A080:10A0BF write change break\n"
             <<"  chqctl why mem A 10A096\n  chqctl changed A 100000:1004FF since 9500\n"
             <<"  chqctl input ports\n  chqctl input pulse 3 20 3\n  chqctl snapshot save trial-a\n";
}

int main(int argc,char** argv){
    std::uint16_t port=37600; int first=1; bool wait_after=false;
    while(first<argc){std::string a=argv[first]; if(a=="--port"){if(first+1>=argc){usage();return 2;}const auto p=std::stoul(argv[first+1]);if(p<1||p>65535){std::cerr<<"invalid port\n";return 2;}port=static_cast<std::uint16_t>(p);first+=2;continue;}if(a=="--wait"){wait_after=true;++first;continue;}break;}
    if(first>=argc){usage();return 0;}
    if(std::string(argv[first])=="shell"){
        std::cout<<"Connected command shell for 127.0.0.1:"<<port<<" (type help, source FILE, quit to exit)\n";
        std::string line; while(std::cout<<"CHQ> " && std::getline(std::cin,line)){if(line=="quit"||line=="exit")break;if(line.empty())continue;
            if(line.rfind("source ",0)==0){if(execute_script(port,line.substr(7),true))std::cerr<<"script stopped\n";continue;}
            execute_one(port,line,true,true);
        } return 0;
    }
    if(std::string(argv[first])=="script"){
        if(first+1>=argc){std::cerr<<"ERR script requires FILE\n";return 2;} bool script_wait=true; if(first+2<argc&&std::string(argv[first+2])=="--no-wait")script_wait=false; return execute_script(port,argv[first+1],script_wait);
    }
    std::ostringstream cmd; for(int i=first;i<argc;++i){if(i>first)cmd<<' ';cmd<<argv[i];}
    return execute_one(port,cmd.str(),wait_after,true);
}
