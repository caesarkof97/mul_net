#include "vnet.h"

static struct net_device *vnet_dev;
static int lockup = 0;
static int timeout = SNULL_TIMEOUT;

static void vnet_rx(struct net_device *dev, int len, unsigned char *buf)
{   
    return;
}

static void vnet_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
    int statusword;
	struct net_device *dev = (struct net_device *)dev_id;
    struct vnet_priv *priv = netdev_priv(dev);
   
    if (!dev) 
		return;
	
    statusword = priv->status;
	priv->status = 0;
    if (statusword & SNULL_RX_INTR) 
	{
        vnet_rx(dev, priv->rx_packetlen, priv->rx_packetdata);
    }
    if (statusword & SNULL_TX_INTR) 
	{
        priv->stats.tx_packets++;
        priv->stats.tx_bytes += priv->tx_packetlen;
        dev_kfree_skb(priv->skb);
    }

    return;
}

static void vnet_hw_tx(char *buf, int len, struct net_device *dev)
{
    struct iphdr *ih;
    struct net_device *dest;
    struct vnet_priv *priv = netdev_priv(dev);
    u32 *saddr, *daddr;

    if (len < sizeof(struct ethhdr) + sizeof(struct iphdr)) 
	{
        printk("snull: Hmm... packet too short (%i octets)\n",len);
        return;
    }
 
    ih = (struct iphdr *)(buf+sizeof(struct ethhdr));
    saddr = &ih->saddr;
    daddr = &ih->daddr;
    ((u8 *)saddr)[2] ^= 1;
    ((u8 *)daddr)[2] ^= 1;
    ih->check = 0;       
    ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);

    if (dev == vnet_dev)
        printk(KERN_INFO"%08x:%05i --> %08x:%05i\n",
               ntohl(ih->saddr),ntohs(((struct tcphdr *)(ih+1))->source),
               ntohl(ih->daddr),ntohs(((struct tcphdr *)(ih+1))->dest));
    else
        printk(KERN_INFO"%08x:%05i <-- %08x:%05i\n",
               ntohl(ih->daddr),ntohs(((struct tcphdr *)(ih+1))->dest),
               ntohl(ih->saddr),ntohs(((struct tcphdr *)(ih+1))->source));

    dest = vnet_dev + (dev==vnet_dev ? 1 : 0);
    priv->status = SNULL_RX_INTR;
    priv->rx_packetlen = len;
    priv->rx_packetdata = buf;
    vnet_interrupt(0, dest, NULL);

    priv->status = SNULL_TX_INTR;
    priv->tx_packetlen = len;
    priv->tx_packetdata = buf;
    if (lockup && ((priv->stats.tx_packets + 1) % lockup) == 0) 
	{
        netif_stop_queue(dev);
        printk(KERN_INFO"Simulate lockup at %ld, txp %ld\n", jiffies,(unsigned long) priv->stats.tx_packets);
    }else{
        vnet_interrupt(0, dev, NULL);
	}
	return;
}


static int vnet_open(struct net_device *dev)
{                    
    netif_start_queue(dev);	
    return 0;
}

static int vnet_release(struct net_device *dev)
{                    
    netif_stop_queue(dev);	
    return 0;
}

static struct net_device_stats *vnet_stats(struct net_device *dev)
{
	struct vnet_priv *priv = netdev_priv(dev);

    return &priv->stats;
}

static int vnet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    return 0;
}


static int vnet_config(struct net_device *dev, struct ifmap *map)
{
    if (dev->flags & IFF_UP)
        return -EBUSY;

    if (map->base_addr != dev->base_addr) 
	{
        printk(KERN_WARNING "vnet: Can't change I/O address\n");
        return -EOPNOTSUPP;
    }

    if (map->irq != dev->irq) 
	{
        dev->irq = map->irq;
    }

    return 0;
}

static netdev_tx_t vnet_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int len;
    char *data;
	netdev_tx_t ret = NETDEV_TX_OK;
	struct vnet_priv *priv = netdev_priv(dev);

    len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;	//当所需传输的数据包长度小于介质所支持的最小长度时，需要小心处理。
    data = skb->data;
    priv->skb = skb;

    vnet_hw_tx(data, len, dev);

    return ret;
}


static int vnet_change_mtu(struct net_device *dev, int new_mtu)
{
    unsigned long flags;
	struct vnet_priv *priv = netdev_priv(dev);
    spinlock_t *lock = &((struct vnet_priv *)priv)->lock;

    if ((new_mtu < 68) || (new_mtu > 1500))	
        return -EINVAL;
  
    spin_lock_irqsave(lock, flags);
    dev->mtu = new_mtu;
    spin_unlock_irqrestore(lock, flags);

    return 0;
}

static void vnet_tx_timeout (struct net_device *dev)
{
	struct vnet_priv *priv = netdev_priv(dev);
    printk(KERN_INFO"Transmit timeout");

    priv->status = SNULL_TX_INTR;
    vnet_interrupt(0, dev, NULL);
    priv->stats.tx_errors++;
    netif_wake_queue(dev);

    return;
}

static const struct net_device_ops dm9000_netdev_ops = {  
	.ndo_open  = vnet_open,
	.ndo_stop  = vnet_release,
	.ndo_get_stats = vnet_stats,
	.ndo_do_ioctl = vnet_ioctl,
	.ndo_set_config = vnet_config,
	.ndo_start_xmit = vnet_xmit,
	.ndo_change_mtu = vnet_change_mtu,
	.ndo_tx_timeout = vnet_tx_timeout,
	
};

static const struct header_ops vnet_header_ops= {
	.create	 = eth_header,
	.parse	 = eth_header_parse,           
};



static int vnet_init(void)
{

	vnet_dev = alloc_netdev_mq(0, "vnet", 'v', ether_setup,1);
	if(!vnet_dev){
		printk("alloc net_device failed!\n");
		return 0;
	}
		
	vnet_dev->netdev_ops = &dm9000_netdev_ops;
	vnet_dev->header_ops = &vnet_header_ops;
	vnet_dev->watchdog_timeo = timeout;
	
	 /* 设置MAC地址 */  
        vnet_dev->dev_addr[0] = 0x08;  
        vnet_dev->dev_addr[1] = 0x89;  
        vnet_dev->dev_addr[2] = 0x89;  
        vnet_dev->dev_addr[3] = 0x89;  
        vnet_dev->dev_addr[4] = 0x89;  
        vnet_dev->dev_addr[5] = 0x11;  
		
	//vnet_dev->flags |= IFF_NOARP;	//禁止ARP
  	vnet_dev->features |= NETIF_F_HW_CSUM;

	register_netdev(vnet_dev);	
	return 0;
}

static void vnet_exit(void)
{	
	if(!vnet_dev)
		return;
	unregister_netdev(vnet_dev);
	free_netdev(vnet_dev);
	printk("vnet stop!\n");
	return;
}

module_init(vnet_init);
module_exit(vnet_exit);

MODULE_AUTHOR("Caesar");
MODULE_LICENSE("GPL");

