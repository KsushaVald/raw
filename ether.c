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
#include <netinet/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
int main(){

	struct sockaddr_in s_addr; struct sockaddr_ll sl_addr;
	struct ifreq ifr, ifr2; char *mac; ;
	int fd_socket; int test, flag=1; int log=0;
	char datagram[1500]; int i;
	char msg[6]="hello\0";
	char *mes_r;
	struct udphdr *udp; struct iphdr *ip; struct ether_header *ether;
	char *packet_s;
	char packet_r[1500];

	socklen_t len=sizeof(struct sockaddr);
	s_addr.sin_family=AF_INET;
	s_addr.sin_port=htons(55555);
	s_addr.sin_addr.s_addr=inet_addr("192.168.2.1");

	memset(datagram,0,1500);
	fd_socket=socket(AF_PACKET,SOCK_RAW,ETH_P_ALL);
	if(fd_socket<0){
		perror("socket");
	}

	ifr2.ifr_addr.sa_family=AF_PACKET;
	strncpy(ifr2.ifr_name,"eth0", IFNAMSIZ-1);
	ioctl(fd_socket,SIOCGIFHWADDR,&ifr2);
	mac=(u_char*)ifr2.ifr_hwaddr.sa_data;
	strncpy(ifr.ifr_name,"eth0", IFNAMSIZ-1);
 	ioctl(fd_socket,SIOCGIFINDEX,&ifr);
	sl_addr.sll_family=AF_PACKET;
	sl_addr.sll_protocol=ETH_P_IP;
	sl_addr.sll_ifindex=ifr.ifr_ifindex;
	sl_addr.sll_hatype=ARPHRD_ETHER;
	sl_addr.sll_pkttype=PACKET_OTHERHOST;
	sl_addr.sll_halen=ETH_ALEN;

	ether=(struct ether_header*)datagram;
	strcpy(ether->ether_dhost,mac);
	strcpy(ether->ether_shost,mac);
	ether->ether_type=ETH_P_IP;
	ip=(struct iphdr*)(datagram+sizeof(struct ether_header));
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
	udp=(struct udphdr*)(datagram+sizeof(struct ether_header)+sizeof(struct iphdr));
	udp->source=htons(35277);
	udp->dest=htons(55555);
	udp->len=htons(sizeof(struct udphdr)+strlen(msg));
	udp->check=0;
	packet_s=malloc(sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(msg));
	memcpy(packet_s,ether,sizeof(struct ether_header));
	memcpy(packet_s+sizeof(struct ether_header),ip,sizeof(struct iphdr));
	memcpy(packet_s+sizeof(struct ether_header)+sizeof(struct iphdr),udp,sizeof(struct udphdr));
	memcpy(packet_s+sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr),msg,strlen(msg));
	if(sendto(fd_socket, packet_s,sizeof(struct ether_header)+htons(ip->tot_len),0,(struct sockaddr*)&sl_addr,sizeof(sl_addr))==-1){
		perror("sendto");
		exit(-1);
	}
	while(1){
		if(recvfrom(fd_socket,&packet_r,1500,0,(struct sockaddr*)&s_addr,&len)==-1){
			perror("recvfrom");
		}
		mes_r=(char*)(packet_r+sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr));
		printf("%s\n", mes_r);
		memset(packet_r,0,1500);
	}
}
