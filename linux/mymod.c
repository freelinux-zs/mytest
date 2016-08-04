#include <linux/module.h>
#include <linux/timer.h>
#include <linux/errno.h>

static int sec = 5;
module_param(sec, int , S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(sec, "set the interval.");

static void mymod_timer(unsigned long data);
static DEFINE_TIMER(timer, mymod_timer, 0, 0);

static void mymod_timer(unsigned long data)
{
	printk(KERN_INFO "mymod: timer\n");
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
