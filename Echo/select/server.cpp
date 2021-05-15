#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <unordered_set>
#include <algorithm>

#define SERV_PORT 8888
#define MAX_LISTEN_QUEUE 5
#define MAX_BUFFER_SIZE 1024

int create_ipv4_tcp_listen_socket() {
    // 创建套接字
    int sockfd;
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        perror("create socket error");
        exit(-1);
    }
    // 将套接字绑定IP地址和端口号
    struct sockaddr_in server;
    bzero(&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(SERV_PORT);
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


int main(int argc, char *argv[]) {
    int listenfd = create_ipv4_tcp_listen_socket();
    // 文件描述符集
    fd_set global_read_fds, current_read_fds;
    // 清空全局文件描述符集
    FD_ZERO(&global_read_fds);
    // 向全局文件描述符集添加监听套接字
    FD_SET(listenfd, &global_read_fds);
    // 所关心的文件描述符的最大值
    int maxfd = listenfd;   // 因为当前只创建了这一个套接字
    // 客户端地址和长度
    struct sockaddr_in client;
    socklen_t len;
    // 客户端套接字集合
    std::unordered_set<int> client_fds;
    // 缓冲区
    char buf[MAX_BUFFER_SIZE];
    // 主程序
    while(true) {
        // 调用select函数
        current_read_fds = global_read_fds; // 因为每次调用select都会修改关心的文件描述符集
        // select 返回 正数：当前有可用的文件描述符，负数：出错，0：超时
        if(select(maxfd+1, &current_read_fds, nullptr, nullptr, nullptr)<0) {
            perror("select error");
            exit(-1);
        }
        // 判断监听套接字当前是否处于可读状态
        if(FD_ISSET(listenfd, &current_read_fds)) {
            // 如果监听套接字为可读
            int sockfd; // 通信套接字
            if((sockfd=accept(listenfd, (struct sockaddr*)&client, &len))<0) {
                perror("accpet error");
                exit(-1);
            }
            FD_CLR(listenfd, &current_read_fds); // 清除监听套接字的可读状态
            client_fds.insert(sockfd);
            FD_SET(sockfd, &global_read_fds); // 现在开始要关心通信套接字是否可读
            maxfd = std::max(maxfd, sockfd);
        }
        // 遍历当前所有客户端套接字，判断是否处于可读状态
        for(auto it=client_fds.begin(); it!=client_fds.end(); ) {
            int fd = *it;
            if(FD_ISSET(fd, &current_read_fds)) {
                bzero(buf, MAX_BUFFER_SIZE);
                int bytes = recv(fd, buf, MAX_BUFFER_SIZE, 0);
                if(bytes<0) {
                    perror("read error");
                    exit(-1);
                }
                if(bytes==0) {
                    printf("Client connection closed.\n");
                    client_fds.erase(it++);
                    close(fd);
                    FD_CLR(fd, &global_read_fds); // 不再关心这个套接字是否可读
                    continue;
                }
                printf("Client: %s\n", buf);
                send(fd, buf, strlen(buf), 0);
            }
            ++it;
        }
    }
    close(listenfd);
    return 0;
}