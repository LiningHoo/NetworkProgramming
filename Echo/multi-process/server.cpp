#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <wait.h>

#define MAX_LISTEN_QUEUE 5
#define PORT 8888

int create_ipv4_tcp_listen_socket() {
    int sockfd;
    // 创建套接字
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        perror("Create socket failed");
        exit(-1);
    }
    // 设置地址可重用
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0) {
        perror("Set socket reuse address failed");
        return -1;
    }
    // 选择IP地址和端口号
    struct sockaddr_in server;
    bzero(&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    // 绑定地址
    if(bind(sockfd, (const sockaddr*)&server, sizeof(sockaddr))<0) {
        perror("Bind address failed");
        exit(-1);
    }
    // 设置套接字为监听套接字
    if(listen(sockfd, MAX_LISTEN_QUEUE)<0) {
        perror("Set listen mode failed");
        exit(-1);
    }
    return sockfd;
}

#define MAX_BUFF_SIZE 100

int echo_server_handler(int sockfd) {
    char buf[MAX_BUFF_SIZE];
    while(true) {
        bzero(buf, sizeof(buf));
        // 从客户端接收请求
        int bytes = recv(sockfd, buf, MAX_BUFF_SIZE, 0);
        if(bytes<0) {
            perror("Recv error");
            break;
        }
        if(bytes==0) {
            printf("Client connnection closed.\n");
            break;
        }
        if(!strcmp(buf, "q")) 
            break;
        if(send(sockfd, buf, strlen(buf), 0)<0) {
            perror("Send error");
            break;
        }
    }
    close(sockfd);
    return 0;
}

void signal_handler(int signo) {
    switch(signo) {
        case SIGCHLD: {
            while(waitpid(-1, nullptr, WNOHANG));
            break;
        }
    }
}

int set_signal_handler() {
    struct sigaction act, oact;
    act.sa_handler = (__sighandler_t)signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_flags |= SA_RESTART;
    if(sigaction(SIGCHLD, &act, &oact)<0)
        return -1;
    return 0;
}

int main(int argc, char *argv[]) {
    int listenfd = create_ipv4_tcp_listen_socket();
    struct sockaddr_in client;
    socklen_t len;
    if(set_signal_handler()<0) {
        perror("Set signal handler error");
        exit(-1);
    }
    while(true) {
        // 休眠进程，直到有客户端连接
        int sockfd = accept(listenfd, (sockaddr*)&client, &len);
        printf("client ip: 0x%x , port:%d\n", ntohl(client.sin_addr.s_addr), ntohs(client.sin_port));
        if(sockfd<0) {
            perror("Accept error");
            exit(-1);
        }
        // 创建子进程
        if(fork()==0) {
            // fork创建的子进程会复制一份父进程的内存空间，所以close(listenfd)将套接字的引用计数减一
            close(listenfd);
            echo_server_handler(sockfd);
            exit(0);
        }
        close(sockfd);
    }
    return 0;
}