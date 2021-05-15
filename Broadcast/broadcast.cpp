/*
网络中通信服务的四种类型：
    单播（unicast）：一对一通信。
    任播（anycast）：IPv6中定义的一种新型通信服务，并未广泛采用。
    组播（multicast）：目的IP地址范围为224.0.0.0 ~ 239.255.255.255
    广播（broadcast）：发送报文到当前子网中所有的主机接口
广播和组播都需要使用UDP，都不能使用UDP。
**/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // 创建UDP套接字
    int sockfd;
    if((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("create socket error");
        exit(-1);
    }
    // 绑定IP地址和端口号
    struct sockaddr_in server;
    bzero(&server, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(8888);
    if(bind(sockfd, (const sockaddr*)&server, sizeof(sockaddr))<0) {
        perror("bind error");
        close(sockfd);
        exit(-1);
    }
    // 设置广播权限
    int on = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0) {
        perror("setsockopt error");
        close(sockfd);
        exit(-1);
    }
    // 广播地址和端口号
    struct sockaddr_in broadcast_addr;
    bzero(&broadcast_addr, sizeof(sockaddr_in));
    broadcast_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "172.26.63.255", &broadcast_addr.sin_addr);
    broadcast_addr.sin_port = htons(8888);  
    // 发送广播数据报
    char buf[] = "Hello, broadcast.";
    if(sendto(sockfd, (const void*)buf, strlen(buf), 0, 
      (const sockaddr*)&broadcast_addr, sizeof(sockaddr))<0) {
        perror("send error");
        close(sockfd);
        exit(-1);
    }
    // 广播数据报会发送给子网上所有的主机，所以自己也能收到
    char read_buf[MAX_BUFFER_SIZE];
    struct sockaddr_in station;
    socklen_t len;
    bzero(read_buf, MAX_BUFFER_SIZE);
    if(recvfrom(sockfd, read_buf, MAX_BUFFER_SIZE, 0, (sockaddr*)&station, &len)<0) {
        perror("recvfrom error");
        close(sockfd);
        exit(-1);
    }
    printf("%s\n", read_buf);
    close(sockfd);
    return 0;
}