#include <linux/module.h>
#include <linux/timer.h>
#include <linux/errno.h>

static int sec = 5;
module_param(sec, int , S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(sec, "set the interval.");

static void mymod_timer(unsigned long data);
static DEFINE_TIMER(timer, mymod_timer, 0, 0);

//static unsigned long bf_time, af_time, bf_all_time, af_all_time;
//static unsigned long time[100]={0};
//int i = 1;

static inline unsigned long get_time(void)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	return (1000000 * tv.tv_sec + tv.tv_usec);
}


static void mymod_timer(unsigned long data)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	printk(KERN_INFO "s = %lu\n",(tv.tv_sec));	
//	printk(KERN_INFO "mymod: timer\n");
//	bf_all_time = get_time();
//	bf_time = get_time();
//	af_time = get_time();
//	af_all_time = get_time();
//	printk(KERN_ALERT "%s:========all_time = %lums, time = %lums\n",__func__,(af_all_time - bf_all_time) / 1000,(af_time - bf_time) / 1000);
//	time[i] = get_time();
//	printk(KERN_ERR"time= %lums\n",(time[i]- time[i-1]) / 1000);
//	if(i != 100)
//		i++;
//	else{
//		i = 1;
//	}
//	printk(KERN_INFO "%s:======time[%d] = %lums\n",__func__,i,(time[i] - time[i-1]));
	mod_timer(&timer,jiffies + sec * HZ);
} 

static int mymod_init(void)
{
	printk(KERN_INFO "mymod: init\n");
	
	if(sec <= 0){
		printk(KERN_INFO "Invalid interval sec=%d\n",sec);
		return -EINVAL;
	}
	
	mod_timer(&timer, jiffies + sec * HZ);
	
	return 0;
}

static void mymod_exit(void)
{
	del_timer(&timer);
	printk(KERN_INFO "mymod: exit\n");
}

module_init(mymod_init);
module_exit(mymod_exit);

MODULE_AUTHOR("Zhoubin Xu");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My module");
