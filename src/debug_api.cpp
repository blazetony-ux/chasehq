#include "debug_api.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
static constexpr socket_t invalid_socket_v = INVALID_SOCKET;
static void close_socket(socket_t s){ if(s!=INVALID_SOCKET) closesocket(s); }
static int last_socket_error(){ return WSAGetLastError(); }
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
static constexpr socket_t invalid_socket_v = -1;
static void close_socket(socket_t s){ if(s>=0) ::close(s); }
static int last_socket_error(){ return errno; }
#endif

namespace chq {

struct DebugApiServer::Impl {
    socket_t listen_socket = invalid_socket_v;
    std::uint16_t port = 0;
#ifdef _WIN32
    bool wsa_started = false;
#endif
};

static bool set_nonblocking(socket_t s){
#ifdef _WIN32
    u_long mode=1; return ioctlsocket(s,FIONBIO,&mode)==0;
#else
    const int flags=fcntl(s,F_GETFL,0); return flags>=0 && fcntl(s,F_SETFL,flags|O_NONBLOCK)==0;
#endif
}

DebugApiServer::DebugApiServer():impl_(std::make_unique<Impl>()){}
DebugApiServer::~DebugApiServer(){ stop(); }

bool DebugApiServer::start(std::uint16_t port, std::string& error){
    stop();
#ifdef _WIN32
    WSADATA w{};
    if(WSAStartup(MAKEWORD(2,2),&w)!=0){ error="WSAStartup failed"; return false; }
    impl_->wsa_started=true;
#endif
    auto s=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(s==invalid_socket_v){ error="socket() failed: "+std::to_string(last_socket_error()); stop(); return false; }
    int yes=1;
#ifdef _WIN32
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&yes),sizeof(yes));
#else
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
#endif
    sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_port=htons(port); addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if(::bind(s,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))!=0){ error="bind(127.0.0.1:"+std::to_string(port)+") failed: "+std::to_string(last_socket_error()); close_socket(s); stop(); return false; }
    if(::listen(s,8)!=0){ error="listen() failed: "+std::to_string(last_socket_error()); close_socket(s); stop(); return false; }
    if(!set_nonblocking(s)){ error="failed to make API socket non-blocking"; close_socket(s); stop(); return false; }
    impl_->listen_socket=s; impl_->port=port; return true;
}

void DebugApiServer::stop(){
    close_socket(impl_->listen_socket); impl_->listen_socket=invalid_socket_v; impl_->port=0;
#ifdef _WIN32
    if(impl_->wsa_started){ WSACleanup(); impl_->wsa_started=false; }
#endif
}

bool DebugApiServer::running() const { return impl_->listen_socket!=invalid_socket_v; }
std::uint16_t DebugApiServer::port() const { return impl_->port; }

void DebugApiServer::poll(const Handler& handler){
    if(!running()) return;
    // Drain a small bounded number per emulation/UI iteration so API use never
    // starves emulation even if a client floods commands.
    for(int accepted=0; accepted<8; ++accepted){
        sockaddr_in peer{};
#ifdef _WIN32
        int peer_len=sizeof(peer);
#else
        socklen_t peer_len=sizeof(peer);
#endif
        socket_t c=::accept(impl_->listen_socket,reinterpret_cast<sockaddr*>(&peer),&peer_len);
        if(c==invalid_socket_v) break;
        std::array<char,8192> buf{};
        std::string request;
        // chqctl sends one short line before waiting for a response. Keep this
        // blocking only on the accepted socket for a bounded single recv.
        const int n=::recv(c,buf.data(),static_cast<int>(buf.size()-1),0);
        if(n>0) request.assign(buf.data(),static_cast<std::size_t>(n));
        auto e=request.find_first_of("\r\n"); if(e!=std::string::npos) request.resize(e);
        std::string response;
        try { response=handler ? handler(request) : "ERR no handler"; }
        catch(const std::exception& ex){ response=std::string("ERR ")+ex.what(); }
        catch(...){ response="ERR unknown exception"; }
        if(response.empty()) response="OK";
        if(response.back()!='\n') response.push_back('\n');
        const char* p=response.data(); std::size_t left=response.size();
        while(left){ const int sent=::send(c,p,static_cast<int>(std::min<std::size_t>(left,1u<<20)),0); if(sent<=0) break; p+=sent; left-=static_cast<std::size_t>(sent); }
        close_socket(c);
    }
}

} // namespace chq
