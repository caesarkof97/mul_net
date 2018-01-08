#include "kernel.h"
#include "source.h"
#include "timer.h"
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NET WORK");
MODULE_AUTHOR("MAC");

static struct nf_hook_ops preRoutHook;//ip包接收钩子点
static struct nf_hook_ops postRoutHook; //ip包发送钩子点
static struct nf_hook_ops arpOutHook; //arp包发送钩子点
int mmHookInit(void)
{
	preRoutHook.hook = preRoutHookDisp;
	preRoutHook.hooknum = NF_INET_PRE_ROUTING;
	preRoutHook.pf = NFPROTO_IPV4;
	preRoutHook.priority = NF_IP_PRI_LAST;
	nf_register_hook(&preRoutHook);
	
	postRoutHook.hook = postRoutHookDisp;
	postRoutHook.hooknum = NF_INET_POST_ROUTING;
	postRoutHook.pf = NFPROTO_IPV4;
	postRoutHook.priority = NF_IP_PRI_LAST;
	nf_register_hook(&postRoutHook);
	
	arpOutHook.hook = arpOutHookDisp;
	arpOutHook.hooknum = NF_ARP_OUT;
	arpOutHook.pf = NFPROTO_ARP;
	arpOutHook.priority = NF_IP_PRI_LAST;
	nf_register_hook(&arpOutHook);
	
	
	return 0;
}


void mmHookExit(void)
{
	nf_unregister_hook(&preRoutHook);
	nf_unregister_hook(&postRoutHook);
	nf_unregister_hook(&arpOutHook);
	
}


static int mm_init(void)
{	
	init_link_info();
	mmHookInit();
	settimer_init();
	printk("car start work!\n");
	return 0;
}


static void mm_exit(void)
{
	settimer_exit();
	mmHookExit();
	exit_link_info();
	printk("car stop work!\n");
}

module_init(mm_init);
module_exit(mm_exit);
