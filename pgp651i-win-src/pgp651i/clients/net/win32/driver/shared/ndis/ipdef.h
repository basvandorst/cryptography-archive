#ifndef _H_IPDEF_
#define _H_IPDEF_

#define		IPPORT				0x0800
#define		IPPROT_NET			0x0008
#define		ARPPROT				0x0806
#define		ARPPROT_NET			0x0608
#define		PROTOCOL_ICMP		1
#define		PROTOCOL_IGMP		2

#define		PROTOCOL_UDP		17
#define		UDP_PORT_IKE		0x01f4		// 500
#define		UDP_PORT_IKE_NET	0xf401

#define MAX_ETHER_FRAME_SIZE	1514
#define	ETHER_HEADER_SIZE 14
#define IP_HEADER_SIZE 20
#define ICMP_HEADER_SIZE 8

#define MAX_TEST_ICMP_DATA_SIZE 100

#define IP_RF 0x80        // reserved
#define IP_DF 0x40        // don't fragment
#define IP_MF 0x20        // more fragments
#define IP_SAVE_FLAGS (IP_RF|IP_MF) // flags saved from packet to each fragment
#define IP_OFFSET ~0xe0    // fragment offset mask

#define IP_MORE_FRAGMENT(x)   ((x) & 0x0020)
#define IP_LAST_FRAGMENT(x)   !(IP_MORE_FRAGMENT(x))

typedef struct tag_ETHERNET_HEADER {

	UCHAR	eth_dstAddress[6];
	UCHAR	eth_srcAddress[6];
	UCHAR	eth_protocolType[2];
	
} ETHERNET_HEADER, *PETHERNET_HEADER;
	
typedef struct tag_IP_HEADER {
    UCHAR ip_headerlength:4;
    UCHAR ip_version:4;
    UCHAR ip_tos;
    USHORT ip_len;
    USHORT ip_id;
    USHORT ip_foff;
    UCHAR ip_ttl;
    UCHAR ip_prot;
    USHORT ip_chksum;
    unsigned long ip_src;
    unsigned long ip_dest;
}IP_HEADER, *PIP_HEADER;

#pragma pack(push, 1)
typedef struct tag_ARP_HEADER {
	USHORT arp_hrd;
	USHORT arp_pro;
	UCHAR  arp_hln;
	UCHAR  arp_pln;
	USHORT arp_op;

	UCHAR  arp_src_hrd_addr[6];
	ULONG  arp_src_ip_addr;
	UCHAR  arp_dst_hrd_addr[6];
	ULONG  arp_dst_ip_addr;

}ARP_HEADER, *PARP_HEADER;
#pragma pack(pop)

typedef struct tag_UDP_HEADER {
   unsigned short source_port;
   unsigned short dest_port;
   unsigned short msg_len;
   unsigned short checksum;
}UDP_HEADER, *PUDP_HEADER;

#define htons(a) ((((a) & 0XFF00) >> 8) | (((a) & 0X00FF) << 8))
#define ntohs(a) (htons(a))
#define htonl(a) ((((a) & 0XFF000000) >> 24) | (((a) & 0X00FF0000) >> 8) | \
		  (((a) & 0X0000FF00) << 8)  | (((a) & 0X000000FF) << 24))
#define ntohl(a) (htonl(a))

#endif //_H_IPDEF_