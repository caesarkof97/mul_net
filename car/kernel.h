#ifndef KERNEL_H
#define KERNEL_H

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
#include <net/ip.h>
#include <uapi/linux/icmp.h>

#endif