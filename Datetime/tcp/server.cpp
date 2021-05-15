#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <ctime>

int main(int argc, char *argv[]) {
    int listenfd;   // 监听套接字
    int sockfd; // 通信套接字
    // 创建监听套接字
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0))<0) {
        perror("Create socket failed");
        return -1;
    }
    // 设置地址可重用
    int opt = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0) {
        perror("Set socket reuse address failed");
        return -1;
    }
    // 将套接字绑定这台主机上的IP地址和端口号
    // 选择要绑定的IP地址和端口号
    struct sockaddr_in server, client;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    // 等价于 server.sin_addr.s_addr = hton
    server.sin_addr.s_addr = inet_addr("172.20.184.252"); // INADDR_ANY表示这台主机上所有可用的IP地址
    printf("inet_addr: 0x%x\n", server.sin_addr.s_addr);
    inet_aton("172.20.184.2520", &server.sin_addr);
    printf("inet_aton: 0x%x\n", server.sin_addr.s_addr);
    inet_pton(AF_INET, "172.20.184.252", (void*)&server.sin_addr);
    printf("inet_pton: 0x%x\n", server.sin_addr.s_addr);
    server.sin_port = htons(8888);
    if(bind(listenfd, (const sockaddr*)&server, sizeof(sockaddr))<0) {
        perror("Bind error");
        return -1;
    }
    #define MAX_LISTEN_QUE 5
    // 监听套接字
    listen(listenfd, MAX_LISTEN_QUE); // 第二个参数为监听队列的长度
    // 这种模式为迭代服务器，每次只服务一个客户端
    while(true) {
        // 睡眠进程，直到有客户端连接
        socklen_t len = sizeof(sockaddr);
        // accept每次返回一个未用的新的套接字用于与新的客户端进行通信
        if((sockfd = accept(listenfd, (sockaddr*)&client, &len))<0) {
            perror("Accept error");
            return -1;
        }
        printf("IP: 0x%x, Port: %d\n", ntohl(client.sin_addr.s_addr), ntohs(client.sin_port));
        sleep(10);
        printf("sockfd: %d\n", sockfd);
        // 向客户端发数据
        char buf[200];  
        const time_t curtime = time(nullptr);
        snprintf(buf, sizeof(buf), "%s", ctime(&curtime));
        write(sockfd, buf, strlen(buf));
        sleep(10);
        // read
        int bytes;
        char read_buf[100];
        bytes = read(sockfd, buf, 100);
        if(bytes<0) {
            printf("read error\n");
            return -1;
        }
        if(bytes==0) {
            printf("client connection closed.\n");
        }
        // 关闭通信套接字，否则套接字会面临枯竭问题
        close(sockfd);
    }
    return 0;
}