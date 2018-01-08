#ifndef VNET_H
#define VNET_H

#include<linux/module.h>
#include<linux/init.h>
#include<linux/netdevice.h>
#include<linux/errno.h>
#include<linux/skbuff.h>
#include<linux/etherdevice.h>
#include<linux/kernel.h>
#include<linux/types.h>//_be32
#include<linux/string.h>
#include<linux/inetdevice.h>
#include<net/net_namespace.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include<linux/udp.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<asm/processor.h>
#include <linux/interrupt.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_arp.h>
#include <uapi/linux/if_arp.h>
#include <net/arp.h>

#define SNULL_RX_INTR 0x0001
#define SNULL_TX_INTR 0x0002
#define SNULL_TIMEOUT 5  

struct vnet_priv {
    struct net_device_stats stats;
	int status;
    int rx_packetlen;
    u8 *rx_packetdata;
    int tx_packetlen;
    u8 *tx_packetdata;
    struct sk_buff *skb;
    spinlock_t lock;
    struct net_device *dev;
};


#endif