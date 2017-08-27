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
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>

static unsigned short checksum(unsigned short *ptr,unsigned int size){
        register unsigned long  sum=0;
        while(size>1){
                sum+=*ptr++;
                size-=2;
        }
        if(size>0){
                sum+=((*ptr)&htons(0xFF00));
        }
        while(sum>>16){
        sum=(sum & 0xffff)+(sum>>16);
        }
        sum=~sum;
        return ((unsigned short)sum);
}


int main(){

	struct sockaddr_in s_addr; struct sockaddr_ll sl_addr;
	struct ifreq ifr;
	int fd_socket; int test, flag=1; int log=0, i;
	u_char mac_s[6]={0x1c,0xb7,0x2c,0x88,0x1b,0x7f};
	u_char mac_d[6]={0x00,0x1e,0x67,0x38,0xd0,0x89};
	u_char mac_ps[6];
	u_char mac_pd[6];
	char datagram[2000];
	char msg[6]="hello\0";
	char *mes_r;
	struct udphdr *udp; struct iphdr *ip; struct ether_header *ether;
	char *packet_s;
	char packet_r[2000];
	socklen_t len=sizeof(struct sockaddr);
	ether=malloc(sizeof(struct ether_header*));
	memset(datagram,0,2000);
	memset(packet_r,0,2000);
	fd_socket=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(fd_socket<0){
		perror("socket");
	}
	if(setsockopt(fd_socket,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
			perror("setsockopt");
	}
	strncpy(ifr.ifr_name,"enp3s0f2", IFNAMSIZ-1);
        ioctl(fd_socket,SIOCGIFINDEX,&ifr);
        sl_addr.sll_family=AF_PACKET;
        sl_addr.sll_protocol=htons(ETH_P_IP);
        sl_addr.sll_ifindex=ifr.ifr_ifindex;
        sl_addr.sll_hatype=ARPHRD_ETHER;
        sl_addr.sll_pkttype=PACKET_OTHERHOST;
        sl_addr.sll_halen=ETH_ALEN;
	//ether=(struct ether_header*)datagram;
	for(i=0; i<6; i++){
		ether->ether_shost[i]=mac_s[i];
		ether->ether_dhost[i]=mac_d[i];
	}
	ether->ether_type=htons(ETH_P_IP);
//	ip=(struct iphdr*)(datagram+sizeof(struct ether_header*));
	ip=malloc(sizeof(struct iphdr));
	ip->version=4;
	ip->ihl=5;
	ip->tos=0;
	ip->tot_len=htons(sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(msg));
	ip->id=htons(35);
	ip->frag_off=0;
	ip->ttl=255;
	ip->protocol=IPPROTO_UDP;
	ip->saddr=inet_addr("192.168.2.61");
	ip->daddr=inet_addr("192.168.2.1");
	ip->check=0;
	ip->check=checksum((unsigned short*)ip,(unsigned int)(ip->ihl<<2));
	udp=malloc(sizeof(struct udphdr));
//	udp=(struct udphdr*)(datagram+sizeof(struct iphdr));
	udp->source=htons(35277);
	udp->dest=htons(55555);
	udp->len=htons(sizeof(struct udphdr)+strlen(msg));
	udp->check=0;
	packet_s=malloc(sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(msg));
	memcpy(packet_s,ether,sizeof(struct ether_header));
	memcpy(packet_s+sizeof(struct ether_header),ip,sizeof(struct iphdr));
	memcpy(packet_s+sizeof(struct ether_header)+sizeof(struct iphdr),udp,sizeof(struct udphdr));
	memcpy(packet_s+sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr),msg,strlen(msg));
	if(sendto(fd_socket, packet_s,sizeof(struct ether_header)+ntohs(ip->tot_len),0,(struct sockaddr*)&sl_addr,sizeof(sl_addr))==-1){
		perror("sendto");
	}
	while(1){
		if(recvfrom(fd_socket,&packet_r,2000,0,(struct sockaddr*)&s_addr,&len)==-1){
			perror("recvfrom");
		}
		mes_r=(char*)(packet_r+sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr));
		printf("%s\n", mes_r);
		memset(packet_r,0,2000);
	}
}
