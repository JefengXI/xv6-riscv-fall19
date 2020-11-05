#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main (int argc, char *argv[]){
    int parentToSonFd[2];
    int sonToParentFd[2];
    char buf[5];
    pipe(parentToSonFd);
    pipe(sonToParentFd);

    int pid = fork();
    //子进程
    if(pid == 0){
        write(sonToParentFd[1],"pong\n",5);
        close(sonToParentFd[1]);
        read(parentToSonFd[0],buf,sizeof(buf));
        printf("%d: received %s",getpid(),buf);
    }
    else{
        write(parentToSonFd[1],"ping\n",5);
        close(parentToSonFd[1]);
        read(sonToParentFd[0],buf,sizeof(buf));
        printf("%d: received %s",getpid(),buf);
    }
    exit();
}