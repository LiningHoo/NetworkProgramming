#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // 创建套接字
    int sockfd;
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        perror("create socket error");
        exit(-1);
    }
    // 指定服务器IP地址和端口号
    struct sockaddr_in server;
    bzero(&server, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "172.26.54.205", &server.sin_addr);
    server.sin_port = htons(8888);
    // 连接服务器
    if(connect(sockfd, (const sockaddr*)&server, sizeof(sockaddr))<0) {
        perror("connect error");
        exit(-1);
    }
    // 缓冲区
    char buf[MAX_BUFFER_SIZE];
    while(true) {
        // 获取用户输入
        bzero(buf, MAX_BUFFER_SIZE);
        fgets(buf, MAX_BUFFER_SIZE, stdin);
        // 向服务器发送数据
        if(send(sockfd, buf, strlen(buf), 0)<0) {
            perror("send error");
            exit(-1);
        }
        // 接收服务器发来的数据
        bzero(buf, MAX_BUFFER_SIZE);
        int bytes = recv(sockfd, buf, MAX_BUFFER_SIZE, 0);
        if(bytes<0) {
            perror("recv error");
            exit(-1);
        }
        if(bytes==0) {
            printf("Server connection closed.\n");
            close(sockfd);
            exit(-1);
        }
        printf("Echo: %s\n", buf);
    }
    close(sockfd);
    return 0;
}