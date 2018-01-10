#include "kernel.h"
#include "source.h"

struct adapter_info adapter_test;
struct link_info link_test;


/***********ip地址转换**********/
static __be32 inet_addr(const char* ip)
{
	int a, b, c, d;
	char addr[4];	
	sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);
	addr[0] = a;
	addr[1] = b;
	addr[2] = c;
	addr[3] = d;	
	return *(__be32 *)addr;
}

/**************MAC IP地址打印**************/
static void show_mac(const char *message , const u8 * mac)
{
	printk("%s: "MAC_FMT, message, MAC_ARG(mac));
	return;
}
static void show_ip(const char *message, const __be32 ip)
{
	printk("%s: "NIPQUAD_FMT, message, NIPQUAD(ip));
	return;
}



/************初始化网卡绑定信息*************/
void init_link_info(void)
{
	u8 dst_mac[6] = {0x44, 0x37, 0xE6, 0xE9, 0x03, 0x61};
	static char *ethName = "ens33";
	/**************配置link信息***************/
	
	link_test.dst_ip = inet_addr("192.168.11.243");    //远端ip
	memcpy(link_test.dst_mac, dst_mac, 6);                     //远端mac
	link_test.adapter = &adapter_test;                 //本地适配器
	
	/**************配置adapter信息*************/
	
	adapter_test.dev = dev_get_by_name(&init_net, ethName);
	
	strcpy(adapter_test.name, adapter_test.dev->name); //本地网卡名
	memcpy(adapter_test.mac, adapter_test.dev->dev_addr, 6);//本地mac地址
	adapter_test.ip = in_dev_get(adapter_test.dev)->ifa_list->ifa_local;//本地ip地址
	return;
}

/*************解除网卡绑定信息*************/
void exit_link_info(void)
{
	dev_put(link_test.adapter->dev);//释放dev
	return;
}

/****************显示网卡绑定信息****************/
static void show_link_info(void)
{
	printk("\n\n远端ip  : "NIPQUAD_FMT, NIPQUAD(link_test.dst_ip));
	printk("远端mac : "MAC_FMT, MAC_ARG(link_test.dst_mac));
	printk("\n本地名称: %s", link_test.adapter->name);
	printk("本地名称: %s", link_test.adapter->dev->name);
	printk("本地ip  : "NIPQUAD_FMT, NIPQUAD(link_test.adapter->ip));
	printk("本地mac : "MAC_FMT, MAC_ARG(link_test.adapter->mac));
	return;
}

/*****************打印dev信息***********************/
static void show_dev(struct net_device *dev)
{
	printk("\n\ndev   name  :\t%s\n", dev->name);//获取网卡名
	printk(    "mac   addr  :\t"MAC_FMT, MAC_ARG(dev->dev_addr));//获取网卡mac地址
	printk(    "perm  addr  :\t"MAC_FMT, MAC_ARG(dev->perm_addr));//获取网卡永久mac
	printk(    "IP    addr  :\t"NIPQUAD_FMT, NIPQUAD(in_dev_get(dev)->ifa_list->ifa_local));//获取网卡IP
	printk(    "broad addr  :\t"NIPQUAD_FMT, NIPQUAD(in_dev_get(dev)->ifa_list->ifa_broadcast));//获取网卡广播
	printk(    "mask  addr  :\t"NIPQUAD_FMT, NIPQUAD(in_dev_get(dev)->ifa_list->ifa_mask));//获取网卡掩码
	return;
}

/******************遍历dev并打印信息********************/
static void get_dev_info(void )
{
	struct net_device *dev;

	read_lock(&dev_base_lock);

	dev = first_net_device(&init_net);
	while (dev) {
		show_dev(dev);
		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);
	return;
}

/******************获取dev******************/
struct link_info * get_useful_dev(void)
{
	return &link_test;
}


/******************构造skb发送指定数据********************/
int new_packet(const char *data)
{
	size_t size = strlen(data);
	struct sk_buff* skb; 
	struct link_info *dst_info;
	dst_info = get_useful_dev();                  //获取有效链路
	if(!dst_info)
		return -1;
	skb = alloc_skb(128+size+16, GFP_ATOMIC);	//帧头+数据+间隙
	if (!skb) {
		
		return -1;
	}
	
	skb_reserve(skb,128);   			//预留帧头
	skb_put(skb,size);      			//扩充数据区
	strcpy(skb->data, data);			//填充数据
	
	send_packet(skb, dst_info);       	//发送udp报文
	
	return 0;
}


/**********************将skb封装成udp报文，发向指定链路***********************/
void send_packet(struct sk_buff *skb, struct link_info *dst_info)
{
	struct udphdr *udph = NULL;
	struct iphdr *iph = NULL;
	struct ethhdr *mach = NULL;
	int length = skb->len;
	
	skb->dev = dst_info->adapter->dev;
	
	udph = (struct udphdr *)skb_push(skb,sizeof(struct udphdr));
	udph->source = htons(UDP_SERV_TEST_PORT);      	//源端口为服务器测试端口
	udph->dest = htons(UDP_CLI_TEST_PORT);          //目的端口为客户测试端口
	udph->len = htons(length + sizeof(struct udphdr));
	udph->check = 0;
	skb_reset_transport_header(skb);
	
	iph =  (struct iphdr *)skb_push(skb,sizeof(struct iphdr));
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = htons(length + sizeof(struct iphdr) + sizeof(struct udphdr));
	iph->id = 0;
	iph->frag_off = 0;
	iph->ttl = 0x21;
	iph->protocol = 0x11;
	iph->check = 0;
	iph->saddr = dst_info->adapter->ip;     //源ip位本地网卡ip
	iph->daddr = dst_info->dst_ip;          //目的ip位远端网卡ip
	iph->check = ip_fast_csum((char *)iph, iph->ihl);
    skb_reset_network_header(skb);  
	

	mach = (struct ethhdr *)skb_push(skb, ETH_HLEN);
	memcpy(mach->h_dest,(unsigned char *)dst_info->dst_mac, ETH_ALEN);				//目的mac为远端mac
	memcpy(mach->h_source,(unsigned char *)dst_info->adapter->mac, ETH_ALEN); 	//源mac为本地网卡mac
	skb->pkt_type = PACKET_OTHERHOST;
	mach->h_proto = htons(ETH_P_IP);         
	skb->protocol = mach->h_proto;
	skb->ip_summed = CHECKSUM_NONE;
	skb_reset_mac_header(skb);
	
	skb->priority = 0;
	skb->_skb_refdst = 0;
	skb->mac_len = ETH_HLEN;	
	
	skb->data_len = 0;
	skb_shinfo(skb)->nr_frags = 0;
	
	skb_set_queue_mapping(skb,2);
	dev_queue_xmit(skb);
	
	return;
}

/*******************preRouting 点钩子函数，截取待接收的IP包，未路由*****************/
unsigned int preRoutHookDisp(void *priv, struct sk_buff *skb, 
				 const struct nf_hook_state *state)
{
	struct udphdr *udph;
	unsigned short innerDstPort;
	char *str_data;
    udph =(struct udphdr *)(skb->data+sizeof(struct iphdr));
    innerDstPort=ntohs(udph->dest);
	str_data = skb->data + sizeof(struct iphdr) + sizeof(struct udphdr);
    if (innerDstPort == UDP_SERV_TEST_PORT)   //处理特定端口
    {
		printk("get udp packet for port UDP_SERV_TEST_PORT\n");
		
		
		/*************************报文处理开始*******************************/
		
		show_link_info();
		printk("preRouting here!\nreceive data: %s\n",str_data);
		new_packet(str_data);
		//get_dev_info();
		
		/*************************报文处理结束**********************************/
		kfree_skb(skb);
		return NF_STOLEN;
	}
    else
		return NF_ACCEPT;
}

/*******************postRouting 点钩子函数，截取待发送的IP包，已路由*****************/
unsigned int postRoutHookDisp(void *priv, struct sk_buff *skb, 
				 const struct nf_hook_state *state)
{
	if(strcmp(skb->dev->name, "vnet"))
		printk("postRouing here!\n");
	return NF_ACCEPT;
}

/****************arpOut 点钩子函数，截取发送的ARP包******************************/
unsigned int arpOutHookDisp(void *priv, struct sk_buff *skb, 
				 const struct nf_hook_state *state)
{
	struct ethhdr *eth;
	struct arphdr *ah;
	u8 mac1[6], mac2[6];
	__be32 ip1, ip2, ip_local;
	u8 *ptr;
	u8 tmp_mac[6] = {0x12, 0x34, 0x56, 0x78, 0x90, 0x66};
	
	ah = (struct arphdr *)(skb->head + skb->network_header);
	eth = (struct ethhdr *) skb -> data;
	ip_local = in_dev_get(skb->dev)->ifa_list->ifa_local;
	
	ptr = (u8 *)(ah+1);
	memcpy(mac1, ptr, 6);
	ptr += 6;
	memcpy(&ip1, ptr, 4);
	ptr += 4;
	memcpy(mac2, ptr, 6);
	ptr += 6;
	memcpy(&ip2, ptr, 4);
	if(ip2==inet_addr("192.168.11.243") || ip2==inet_addr("192.168.11.2"))//跳过远程ssh连接的主机和测试服务器
		return NF_ACCEPT;
		
	printk("\n\n");
	show_mac("发送mac 地址", mac1);
	show_ip( "发送ip  地址", ip1 );
	show_mac("目标mac 地址", mac2);
	show_ip( "目标ip  地址", ip2 );	
	
	/****************处理arp reply*****************/
	skb_pull(skb, ETH_HLEN);
	ah->ar_op = htons(ARPOP_REPLY);
	
	ptr = (u8 *)(ah+1);
	memcpy(ptr, tmp_mac, 6);
	ptr += 6;
	memcpy(ptr, &ip2, 4);
	ptr += 4;
	memcpy(ptr, mac1, 6);
	ptr += 6;
	memcpy(ptr, &ip1, 4);
	
	
	netif_receive_skb(skb);
	printk("arp here!\n自定义arp包\n");
	
	return NF_STOLEN;
}
