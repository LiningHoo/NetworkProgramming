#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    // 创建UDP套接字
    int sockfd;
    if((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("Create socket error");
        return -1;
    }
    // 绑定本机IP地址和端口号
    struct sockaddr_in server;
    bzero(&server, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server.sin_addr);
    server.sin_port = htons(8888);
    if(bind(sockfd, (const sockaddr*)&server, sizeof(sockaddr))<0) {
        perror("Bind error");
        return -1;
    }
    // 等待客户端发来请求
    int bytes;
    char read_buf[100];
    sockaddr_in client;
    socklen_t len = sizeof(sockaddr);
    while(true) {
        bytes = recvfrom(sockfd, read_buf, 100, 0, (sockaddr*)&client, &len);
        if(bytes<0) {
            printf("Recv error\n");
            close(sockfd);
            return -1;
        }
        // 获取当前时间
        time_t t = time(nullptr);
        char buf[200];
        snprintf(buf, sizeof(buf), "%s", ctime(&t));
        // 将时间发送给客户端
        sendto(sockfd, buf, strlen(buf), 0, (const sockaddr*)&client, len);
        // 清空缓冲区
        bzero(&client, len);
        bzero(read_buf, sizeof(read_buf));
        bzero(buf, sizeof(buf));
    }
    // 关闭套接字
    close(sockfd);
    return 0;
}