#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

int main(){
        int fd_socket; int test;
        struct sockaddr_in my_addr, client;
        socklen_t len=sizeof(struct sockaddr);
        char msg[256]="Hi!\0"; char buf[256];
	memset(buf,0,256);
	my_addr.sin_family=AF_INET;
        my_addr.sin_port=htons(55555);
        my_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	fd_socket=socket(AF_INET,SOCK_DGRAM,0);
	if(bind(fd_socket,(struct sockaddr*)&my_addr,sizeof(my_addr))==-1){
                perror("bind");
        }
	recvfrom(fd_socket,&buf,256,0,(struct sockaddr*)&client,&len);
	printf("%s\n", buf);
	sendto(fd_socket,msg,strlen(msg),0,(struct sockaddr*)&client,len);
}




