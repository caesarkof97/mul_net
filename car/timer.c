#include "kernel.h"
#include "timer.h"

static struct hrtimer timer;
static ktime_t kt;
static struct work_struct net_work;

/********************定时器处理*******************/
static enum hrtimer_restart hrtimer_handler(struct hrtimer *timer)
{
	schedule_work(&net_work);
	
	hrtimer_forward(timer, timer->base->get_time(), kt);
	printk("timer work here!");
	return HRTIMER_RESTART;
}



static void packet_process(struct work_struct *work)
{
	printk("packet process here!\n");
	return;
}

int settimer_init(void)
{
	int secs = 30;
	int nsecs = 0;
	kt = ktime_set(secs, nsecs); /*secs seconds + nsecs nanoseconds*/
	
	INIT_WORK(&net_work, packet_process);
	
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_start(&timer, kt, HRTIMER_MODE_REL);
	timer.function = hrtimer_handler;
	printk("\n-------- timer start ---------");
	
	
	
	return 0;
}

int settimer_exit(void)
{
	hrtimer_cancel(&timer);
	msleep(1000);
	destroy_work_on_stack(&net_work);
	printk("-------- timer over ----------");
	return 0;
	
}
