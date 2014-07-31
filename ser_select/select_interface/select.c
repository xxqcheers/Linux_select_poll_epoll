#include"select.h"
#include"network.h"
#include<sys/types.h>
#include<sys/socket.h>

void select_add_fd(select_t *sel,int fd);
void select_del_fd(select_t *sel,int i);


void select_init(select_t *sel,int listenfd)
{
    int i;
    sel->listenfd_=listenfd;
    for(i=0;i<FD_SETSIZE;i++)
        sel->clients_[i]=-1;
    sel->maxi_=0;
    sel->maxfd_=listenfd;
    FD_ZERO(&sel->allset_);
    FD_ZERO(&sel->rset_);
    FD_SET(sel->listenfd_,&sel->allset_);
}

int select_do_wait(select_t *sel)
{
    //注意这个函数的代码很可能会出现错误
    sel->rset_=sel->allset_;

    int nready;
    do{
        nready=select(sel->maxfd_+1,&sel->rset_,NULL,NULL,NULL);
    }while(nready==-1&&errno==EINTR);

    if(nready==-1)
        ERR_EXIT("select");
    sel->nready_=nready;
    return nready;
    /*sel->nready_=select(sel->maxfd_+1,&sel->rset_,NULL,NULL,NULL);
    if(sel->nready_==-1)
    {
        if(errno==EINTR)
            continue;
        else
            ERR_EXIT("select");
    }
    if(sel->nready_==0)
    {
        continue;
    }
    return sel->nready_;
    */
}

void select_handle_accept(select_t *sel)
{
    struct sockaddr_in peeraddr;
    memset(&peeraddr,0,sizeof(peeraddr));
    int len=sizeof(peeraddr);
    if(FD_ISSET(sel->listenfd_,&sel->rset_))
    {
        int peerfd=accept(sel->listenfd_,(struct sockaddr*)&peeraddr,&len);
        if(peerfd==-1)
        {
            ERR_EXIT("accept");
        }
        select_add_fd(sel,peerfd);
    }
}

void select_handle_data(select_t *sel)
{
    int i;
    for(i=0;i<=sel->maxi_;i++)
    {
        if(FD_ISSET(sel->clients_[i],&sel->rset_))
        {
            char recvbuf[1024]={0};
            int fd=sel->clients_[i];
            int ret=readline(fd,recvbuf,1024);
            if(ret==-1)
            {
                ERR_EXIT("readline");
            }
            else if(ret==0)
            {
                printf("client close\n");
                select_del_fd(sel,i);
                continue;
            }
            else
            {
                //处理数据
                writen(fd,recvbuf,strlen(recvbuf));
            }
        }
    }
}

void select_add_fd(select_t *sel,int fd)
{
    int i;
    for(i=0;i<FD_SETSIZE;++i)
    {
        if(sel->clients_[i]==-1)
        {
            sel->clients_[i]=fd;
            if(i>sel->maxi_)
                sel->maxi_=i;
            break;
        }
    }
    if(i==FD_SETSIZE)
    {
        fprintf(stderr,"too many clients\n");
        exit(EXIT_FAILURE);
    }
    FD_SET(fd,&sel->allset_);
    if(fd>sel->maxfd_)
        sel->maxfd_=fd;
}

void select_del_fd(select_t *sel,int i)
{
    assert(i>=0&&i<FD_SETSIZE);
    int fd=sel->clients_[i];
    sel->clients_[i]=-1;
    FD_CLR(fd,&sel->allset_);
    close(fd);
}
