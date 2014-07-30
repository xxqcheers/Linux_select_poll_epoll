#include "network.h"
#include <sys/epoll.h>


void do_service(int peerfd)
{
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};

    //准备工作
    //1.创建epoll句柄，fd
    int epollfd = epoll_create(2);  
    if(epollfd == -1)
        ERR_EXIT("epoll_create");
    //2.注册两个fd
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
        ERR_EXIT("epoll_ctl");
    ev.events = EPOLLIN;
    ev.data.fd = peerfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, peerfd, &ev) == -1)
        ERR_EXIT("epoll_ctl");

    //3.准备一个数组
    struct epoll_event events[2];
    int nready;

    while(1)
    {
        //int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout); 
        nready = epoll_wait(epollfd, events, 2, -1);
        if(nready == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("epoll_wait");
        }else if(nready == 0)
            continue;


        //开始检查每个fd 
        int i;
        for(i = 0; i < nready ; ++i)
        {
            int fd = events[i].data.fd;
            if(fd == STDIN_FILENO)
            {
                if(fgets(sendbuf, 1024, stdin) == NULL)
                {
                    shutdown(peerfd, SHUT_WR);
                    //移除stdin这个fd
                    struct epoll_event ev;
                    ev.data.fd = STDIN_FILENO;
                    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, STDIN_FILENO, &ev) == -1)
                        ERR_EXIT("Epoll_ctl");
                }else
                {
                    writen(peerfd, sendbuf, strlen(sendbuf));
                }
            }

            if(fd == peerfd)
            {
                int ret = readline(peerfd, recvbuf, 1024);
                if(ret == -1)
                    ERR_EXIT("readline");
                else if(ret == 0)
                {
                    close(peerfd);
                    printf("server close\n");
                    exit(EXIT_SUCCESS);
                }
                printf("recv data: %s", recvbuf);
            }


        }
    }



}




int main(int argc, const char *argv[])
{
    //创建socket
    int peerfd = socket(PF_INET, SOCK_STREAM, 0);
    if(peerfd == -1)
        ERR_EXIT("socket");


    //connect
    struct sockaddr_in peeraddr;
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(8989);
    peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(peerfd, (struct sockaddr *)&peeraddr, sizeof peeraddr) < 0)
        ERR_EXIT("connect");



    do_service(peerfd);


    close(peerfd);

    return 0;
}
