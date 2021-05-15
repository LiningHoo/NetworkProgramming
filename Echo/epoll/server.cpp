#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#define SERV_PORT 8888
#define MAX_LISTEN_QUEUE 5

int create_ipv4_tcp_listen_socket() {
    // 创建套接字
    int sockfd;
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        perror("create socket error");
        exit(-1);
    }
    // 指定IP地址和端口号
    struct sockaddr_in server;
    bzero(&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(8888);
    // 将套接字绑定IP地址和端口号
    if(bind(sockfd, (const sockaddr*)&server, sizeof(sockaddr))<0) {
        perror("bind error");
        exit(-1);
    }
    // 设置套接字为监听套接字
    if(listen(sockfd, MAX_LISTEN_QUEUE)<0) {
        perror("listen error");
        exit(-1);
    }
    return sockfd;
}

#define MAX_BUFFER_SIZE 8
#define RD_ERR (-1)
#define CLIENT_CLOSED (-2)
#define WR_ERR (-3)
#define SUCCESS 0

// 处理缓冲区不够一次读完客户端发来的数据的情况（循环读取）
int echo_service(int sockfd) {
    char buf[MAX_BUFFER_SIZE];
    std::string s;
    // 接收请求
    bool readable = true;
    while(readable) {
        bzero(buf, MAX_BUFFER_SIZE);
        int bytes = recv(sockfd, buf, MAX_BUFFER_SIZE, 0);
        if(bytes<0) {
            // 对于非阻塞套接字，已经读完，再次读取，返回-1，且设置错误码errno=EAGAIN
            if(errno==EAGAIN)
                break;
            perror("read error");
            return RD_ERR;
        }
        if(bytes==0) {
            printf("client connection closed.\n");
            return CLIENT_CLOSED;
        }
        if(bytes<MAX_BUFFER_SIZE)
            readable = false;
        s += buf;
    }
    printf("Client: %s\n", s.c_str());
    // 响应
    if(send(sockfd, s.c_str(), s.length(), 0)<0) {
        perror("send error");
        return WR_ERR;
    }
    return SUCCESS;
}

#define MAX_EVENTS 500

int main(int argc, char *argv[]) {
    int listenfd = create_ipv4_tcp_listen_socket();
    // 创建epoll对象
    int epollfd = epoll_create(MAX_EVENTS); // 参数为被监听事件的最大数量
    if(epollfd<0) {
        perror("epoll_create error");
        exit(-1);
    }
    // 将listenfd的读事件添加到epoll对象中
    epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    // 监听套接字listenfd上的EPOLLIN事件
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev)<0) {
        perror("epoll_ctl error");
        exit(-1);
    }
    // 存放就绪事件的数组
    epoll_event events[MAX_EVENTS];
    // 客户端地址和长度
    struct sockaddr_in client;
    socklen_t len;
    // 主程序
    while(true) {
        int evnum = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if(evnum<0) {
            perror("epoll_wait error");
            exit(-1);
        }
        for(int i=0; i<evnum; ++i) {
            if(events[i].data.fd==listenfd) {
                // accept将进程置于睡眠状态，直到有客户端连接，返回对这个客户端的通信套接字
                int sockfd = accept(listenfd, (sockaddr*)&client, &len);
                if(sockfd<0) {
                    perror("accept error");
                    continue;
                }
                // 设置通信套接字为非阻塞
                // 因为epoll边沿触发模式下，只有在缓冲区增加数据时才会触发事件，
                // 此时要将缓冲区中的内容一次读完，这以recv返回EAGAIN为标志
                // 如果套接字是可阻塞的，那么程序会阻塞在这个套接字的读取上，对其他事件的处理产生影响
                int old_mode = fcntl(sockfd, F_GETFD);
                int new_mode = old_mode | O_NONBLOCK;
                fcntl(sockfd, F_SETFD, new_mode);
                // 将sockfd的EPOLLIN事件加入epoll对象中
                ev.data.fd = sockfd;
                ev.events = EPOLLIN | EPOLLET; // EPOLLIN -> 读取事件，EPOLLET -> 边沿触发（edge trigger）
                epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
            } else {
                int status = echo_service(events[i].data.fd);
                if(status==RD_ERR) 
                    continue;
                if(status==CLIENT_CLOSED) {
                    // 不再关心这个套接字的读取事件
                    ev.data.fd = events[i].data.fd;
                    ev.events = EPOLLIN | EPOLLET;
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                    // 关闭这个通信套接字
                    close(events[i].data.fd);
                }
            }
        }
    }
    close(epollfd); // epoll对象句柄也属于套接字，需要关闭，避免出现套接字资源枯竭的情况
    close(listenfd);
    return 0;
}