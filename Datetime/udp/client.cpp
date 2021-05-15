#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>


int main(int argc, char *argv[]) {
    // 创建UDP套接字
    int sockfd;
    if((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("Create socket error");
        return -1;
    }
    // 服务器地址
    struct sockaddr_in server;
    bzero(&server, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "172.20.184.252", &server.sin_addr);
    server.sin_port = htons(8888);
    // 向服务器发送请求
    char request[] = "Get Current Time";
    int bytes = sendto(sockfd, request, sizeof(request), 0, (const sockaddr*)&server, sizeof(sockaddr));
    if(bytes<0) {
        printf("Send error.\n");
        close(sockfd);
        return -1;
    }
    // 接收响应数据
    char read_buf[200];
    bzero(read_buf, sizeof(read_buf));
    socklen_t len = sizeof(sockaddr);
    bytes = recvfrom(sockfd, read_buf, sizeof(read_buf), 0, (sockaddr*)&server, &len);
    if(bytes<0) {
        printf("Read error.\n");
        close(sockfd);
        return -1;
    }
    // 显示从服务器收到的数据
    printf("%s\n", read_buf);
    // 关闭套接字
    close(sockfd);
    return 0;
}