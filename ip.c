#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pcap/pcap.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
int main(){

	struct sockaddr_in s_addr;
	int fd_socket; int test, flag=1; int log=0;
	char datagram[1500];
	char msg[6]="hello\0";
	char *mes_r;
	struct udphdr *udp; struct iphdr *ip;
	char *packet_s;
	char packet_r[1000];
	socklen_t len=sizeof(struct sockaddr);
	s_addr.sin_family=AF_INET;
	s_addr.sin_port=htons(55555);
	s_addr.sin_addr.s_addr=inet_addr("192.168.2.1");
	memset(datagram,0,1500);
	fd_socket=socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
	if(fd_socket<0){
		perror("socket");
	}
	if(setsockopt(fd_socket, IPPROTO_IP,IP_HDRINCL,&flag,sizeof(flag))){
			perror("setsockopt");
	}
	ip=(struct iphdr*)datagram;
	ip->version=4;
	ip->ihl=5;
	ip->tos=0;
	ip->tot_len=htons(sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(msg));
	ip->id=htons(35);
	ip->frag_off=0;
	ip->ttl=0;
	ip->protocol=IPPROTO_UDP;
	ip->check=0;
	ip->saddr=inet_addr("192.168.2.1");
	ip->daddr=inet_addr("192.168.2.1");
	udp=(struct udphdr*)(datagram+sizeof(struct iphdr));
	udp->source=htons(35277);
	udp->dest=htons(55555);
	udp->len=htons(sizeof(struct udphdr)+strlen(msg));
	udp->check=0;
	packet_s=malloc(sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(msg));
	memcpy(packet_s,ip,sizeof(struct iphdr));
	memcpy(packet_s+sizeof(struct iphdr),udp,sizeof(struct udphdr));
	memcpy(packet_s+sizeof(struct iphdr)+sizeof(struct udphdr),msg,strlen(msg));
	if(sendto(fd_socket, packet_s,htons(ip->tot_len),0,(struct sockaddr*)&s_addr,sizeof(s_addr))==-1){
		perror("sendto");
	}
	while(1){
		if(recvfrom(fd_socket,&packet_r,1000,0,(struct sockaddr*)&s_addr,&len)==-1){
			perror("recvfrom");
		}
		mes_r=(char*)(packet_r+sizeof(struct iphdr)+sizeof(struct udphdr));
		printf("%s\n", mes_r);
		memset(packet_r,0,1000);
	}
}
