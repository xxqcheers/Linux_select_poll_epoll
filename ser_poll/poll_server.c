#include "network.h"
#include <poll.h>

int get_listen_fd();
void do_poll(int listenfd);

int main(int argc, const char *argv[])
{
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        ERR_EXIT("signal");
    
    int listenfd = get_listen_fd();

    do_poll(listenfd);

    close(listenfd);
    return 0;
}



int get_listen_fd()
{
    //创建socket
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
        ERR_EXIT("socket");


    //设置端口复用
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8989);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind端口
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof servaddr) < 0)
        ERR_EXIT("bind"); 

    //listen端口
    if(listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");

    return listenfd;
}



void do_poll(int listenfd)
{
    struct pollfd clients[2048];
    int i;
    for(i = 0; i < 2048; ++i)
    {
        clients[i].fd = -1;
    }
    clients[0].fd = listenfd;
    clients[0].events = POLLIN;
    int max = 0;  //最大数组下标
    int nready;
    

    while(1)
    {
        //poll
        nready = poll(clients, max+1, -1);
        if(nready == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("poll");
        }else if(nready == 0)
            continue;
        //listenfd
        if(clients[0].revents & POLLIN)
        {
            int peerfd = accept(clients[0].fd, NULL, NULL);
            if(peerfd == -1)
                ERR_EXIT("accept");
            
            int i = 1;
            for(i = 1; i < 2048; ++i)
            {
                if(clients[i].fd == -1)
                {
                    clients[i].fd = peerfd;
                    clients[i].events = POLLIN;
                    if(i > max)
                        max = i;
                    break;
                }
            }
            if(i == 2048)
            {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }
            if(--nready <= 0)
                continue;
        }

        //clients

        int i;
        for(i = 1; i <= max; ++i)
        {
            int fd = clients[i].fd;
            if(fd == -1)
                continue;

            char recvbuf[1024] = {0};
            if(clients[i].revents & POLLIN)
            {
                int ret = readline(fd, recvbuf, 1024);
                if(ret == -1)
                    ERR_EXIT("readline");
                else if(ret == 0)
                {
                    close(fd);
                    clients[i].fd = -1;
                    continue;
                }

                printf("recv data: %s", recvbuf);
                writen(fd, recvbuf, strlen(recvbuf));

                if(--nready <= 0)
                    break;
            }
        }


    }

}
