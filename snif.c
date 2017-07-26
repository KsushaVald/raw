#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pcap/pcap.h>

int main(){

	struct sockaddr_in s_addr;
	int fd_socket; int test, flag=1;
	char datagram[1500];
	char msg[6]="hello\0";
	char *mes_r;
	struct udphdr *udp;
	char *packet_s, *packet_d;
	s_addr.sin_family=AF_INET;
	s_addr.sin_port=htons(35276);
	s_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	memset(datagram,0,1500);
	fd_socket=socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
	if(setsockopt(fd_socket, IPPROTO_IP,IP_HDRINCL,&flag,sizeof(flag))){
			perror("setsockopt");
	}
	udp=(struct udphdr*)datagram;
	udp->sourse=htons(35277);
	udp->dest=htons(35276);
	udp->len=htons(sizeof(struct udphdr)+sizeof(msg));
	udp->check=0;
	packet_s=malloc(sizeof(struct udphdr)+sizeof(msg));
	memcpy(packet_s,udp,sizeof(struct udphdr));
	memcpy(packet_s+sizeof(struct udphdr),msg,sizeof(msg));
	if(connect(fd_socket,(struct sockaddr*)&s_addr,sizeof(s_addr))==-1){
			perror("connect");
	}
	send(fd_socket, packet_s,udp->len,0);
	recv(fd_socket, packet_r,udp->len,0);
	mes_r=(char*)(paket_r+sizeof(struct udphdr));
	printf("%s\n", mes_r);
}
