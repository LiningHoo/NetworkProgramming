#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

// 客户端程序
int main(int argc, char *argv[]) {
    // 创建套接字
    int sockfd;
    // 第三个参数为协议类型，如果为0表示由操作系统内核根据前两个参数自动推导
    // 三个参数： 地址族(AF_INET/AF_INET6) 套接字类型(SOCK_STREAM/SOCK_DGRAM) 协议(IPPROTO_TCP/IPPROTO_UDP)
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0) {
        // 返回值为-1表示创建套接字失败
        perror("socket error");
        return -1;
    } 
    // 指定服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr)); // 结构体置零
    servaddr.sin_family = AF_INET;  
    // 将点分十进制的字符串形式IP地址转换成无符号整型在网络中传输
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(8888); // 主机字节序转换成网络字节序
    // 连接服务器
    if(connect(sockfd, (const sockaddr*)&servaddr, sizeof(servaddr))<0) {
        perror("connect error");
        return -1;
    }
    // 从服务器读取数据
    char buf[100]; // 用于接收数据的缓冲区
    bzero(buf, sizeof(buf));
    int bytes;  // 读到数据的长度
    bytes = read(sockfd, buf, 100);
    if(bytes<0) {
        printf("Error: read failed.\n");
        return -1;
    }
    // bytes==0表示连接中断
    if(bytes==0) {
        printf("Server close connection.\n");
        return -1;
    }
    // 显示数据
    printf("Read bytes: %d\n", bytes);
    printf("Time: %s\n", buf);
    // 关闭套接字
    close(sockfd);
    return 0;
}