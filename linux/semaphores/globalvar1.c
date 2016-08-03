#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
//#include <asm/semaphore.h>
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");

#define MAJOR_NUM 247

static ssize_t globalvar_read(struct file *, char *, size_t, loff_t *);
static ssize_t globalvar_write(struct file *, const char *, size_t, loff_t *);

struct file_operations globalvar_fops = {
	.read = globalvar_read,
	.write = globalvar_write,
};

static int global_var = 0;
static struct semaphore sem;
static dev_t devno;

static int __init globalvar_init(void)
{
	int ret;
	devno = MKDEV(MAJOR_NUM, 0);
//	ret = register_chrdev(MAJOR_NUM,"globalvar1",&globalvar_fops);
	ret = register_chrdev_region(devno, 1, "globalvar1");
	if(ret){
		printk(KERN_INFO "globalvar: register failure\n");
	}else{
		printk(KERN_INFO "globalvar: register success\n");
		//init_MUTEX(&sem);
		sema_init(&sem, 0);
	}

	return ret;
}

static void __exit globalvar_exit(void)
{
	//unregister_chrdev(MAJOR_NUM,"globalvar1");
	unregister_chrdev_region(MKDEV(MAJOR_NUM, 0), 1);
}

static ssize_t globalvar_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	if(down_interruptible(&sem)){
		return -ERESTARTSYS;
	}

	if(copy_to_user(buf, &global_var, sizeof(int))){
		up(&sem);
		return -EFAULT;
	}

	up(&sem);

	return sizeof(int);

}


static ssize_t globalvar_write(struct file *flip, const char *buf, size_t len, loff_t *off)
{
	if(down_interruptible(&sem)){
		return -ERESTARTSYS;
	}

	if(copy_from_user(&global_var, buf, sizeof(int))){
		up(&sem);
		return -EFAULT;
	}
	up(&sem);
	return sizeof(int);
}

module_init(globalvar_init);
module_exit(globalvar_exit);
