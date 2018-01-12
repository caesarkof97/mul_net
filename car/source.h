#ifndef SOURCE_H
#define SOURCE_H

/**
 *	@CLI:	PC
 *  @SERV:	CAR
*/
#define UDP_CLI_TEST_PORT 4045
#define UDP_SERV_TEST_PORT 7006


/*************打印IP地址*****************/
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2],  \
        ((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u\n"

/*************打印MAC地址*****************/
#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],\
					((u8*)(x))[2],((u8*)(x))[3],\
					((u8*)(x))[4],((u8*)(x))[5]
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x\n"

/*****************绑定的网卡******************/
struct adapter_info {
	char name[16]; 					//网卡名称
	struct net_device *dev;			//网卡对应的net_device结构
	u8 mac[6];						//网卡MAC地址
	__be32 ip;						//网卡IP地址
};

struct  link_info {
	__be32 dst_ip;						//远端IP
	u8 dst_mac[6];					//远端MAC
	struct adapter_info *adapter;	//本地网络适配器
};

void init_link_info(void);
void exit_link_info(void);
struct link_info * get_useful_dev(void);
int new_packet(const char *data);
void send_packet(struct sk_buff *skb, struct link_info *dst_info);
unsigned int preRoutHookDisp(void *priv, struct sk_buff *skb, 
				 const struct nf_hook_state *state);
unsigned int postRoutHookDisp(void *priv, struct sk_buff *skb, 
				 const struct nf_hook_state *state);
unsigned int forwardHookDisp(void *priv, struct sk_buff *skb, 
				const struct nf_hook_state *state);
unsigned int arpOutHookDisp(void *priv, struct sk_buff *skb, 
				 const struct nf_hook_state *state);				 

#endif
