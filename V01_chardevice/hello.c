#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/sysfs.h> 
#include <linux/hrtimer.h>  
#include <linux/ktime.h>
#include <linux/dma-mapping.h> //DO-->hrtimer包含以下三个头文件 /* DMA APIs             */   
#include <linux/time.h>           /* struct timespec    */


#ifndef MEMDEV_MAJOR
#define MEMDEV_MAJOR 0 /*预设的mem的主设备号*/
#endif

#ifndef MEMDEV_NR_DEVS
#define MEMDEV_NR_DEVS 2 /*设备数*/
#endif

#ifndef MEMDEV_SIZE
#define MEMDEV_SIZE 4096
#endif


/*mem设备描述结构体*/
struct mem_dev{
	char *data;
	unsigned long size;
};


static int mem_major = MEMDEV_MAJOR;
struct mem_dev *mem_devp; /*设备结构体指针*/
struct cdev cdev;
static struct class *cdev_class = NULL;
static struct device *cdev_device = NULL;
static dev_t devno;
/*timer*/
struct hrtimer timer;
static ktime_t ktime;  
static unsigned int interval=5000; /* unit: us */  
struct timespec uptimeLast;
static struct work_struct mem_work;

#define TEST_IOCTL_BASE 99
#define TEST_IOCTL_0    _IO(TEST_IOCTL_BASE,0)
#define TEST_IOCTL_1    _IOW(TEST_IOCTL_BASE,1,int)


static ssize_t mem_get_reg(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t mem_set_reg(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);
static ssize_t mem_set_debug(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);

static DEVICE_ATTR(reg, S_IRUGO | S_IWUSR,mem_get_reg,  mem_set_reg);
static DEVICE_ATTR(debug, S_IRUGO | S_IWUSR,NULL,  mem_set_debug);
module_param(mem_major, int, S_IRUGO);

static ssize_t mem_get_reg(struct device* cd,struct device_attribute *attr, char* buf){
	unsigned char reg_val = 10;
	ssize_t len = 0;
	int i  = 10;
	len += snprintf(buf+len, PAGE_SIZE-len, "reg%X = 0x%X,\n", i,reg_val);
	return len;
}

static ssize_t mem_set_reg(struct device* cd, struct device_attribute *attr, const char* buf, size_t len){
	unsigned int databuf[2];
	if(2 == sscanf(buf,"%x %x",&databuf[0], &databuf[1]))
	{
	printk("data = %x %x\n",databuf[0],databuf[1]);
	}
	return len;
}

static ssize_t mem_set_debug(struct device* cd, struct device_attribute *attr, const char* buf, size_t len){
	unsigned int databuf[16];
	sscanf(buf,"%d",&databuf[0]);
	if(databuf[0] == 0) {		// OFF
		printk("off devices \n");
	} else if(databuf[0] == 1) {	//ON
		sscanf(buf, "%d %d %d", &databuf[1], &databuf[2], &databuf[3]);
		printk("open device \n ");
	} else if(databuf[0] == 2) {	//Blink
		sscanf(buf, "%d %d %d %d %d %d", &databuf[1], &databuf[2], &databuf[3], &databuf[4], &databuf[5], &databuf[6]);
		printk("set device one mode\n");
	} else if(databuf[0] == 3) {	//Audio
		sscanf(buf, "%d", &databuf[1]);
		printk("set device two mode\n");
	}

	return len;
}

/*文件打开函数*/
int mem_open(struct inode *inode, struct file *filp)
{
	printk("mem_open\n");
	return 0;
}

 

/*文件释放函数*/
int mem_release(struct inode *inode, struct file *filp)
{
	printk("mem_release\n");
	return 0;
}


/*读函数*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	int a = 100;
	if(copy_to_user(buf,&a,sizeof(int)))
		return -EINVAL;
		
		printk("mem_read\n");
	return 0;
}

 

/*写函数*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	int a ;
	if(copy_from_user(&a, buf, sizeof(int)))
		return -EINVAL;
		
 	printk("mem_write a = %d,\n",a);
 	 return 0;
}

 

/* seek文件定位函数 */
static loff_t mem_llseek(struct file *filp, loff_t offset, int whence)
{
		printk("mem_llseek\n");
		return 0;
}


static long mem_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int test_date;
	printk("cmd = %d\n",cmd);
	switch (cmd){
		case TEST_IOCTL_0:
			printk("case 0\n");
			break;
		case TEST_IOCTL_1:
			printk("case 1\n");
			if(copy_from_user(&test_date,argp,sizeof(test_date)))
				return -EINVAL;
			printk("test_dates = %d\n",test_date);
			break;
		default:
			printk("error\n");
			return -EINVAL;
	}

	return 0;
}

static void mem_work_func(struct work_struct *work)
{        
	hrtimer_start(&timer,
            ktime_set(2000 / 1000, (2000 % 1000) * 1000000),HRTIMER_MODE_REL);  //2S 超时 
}

/*time function*/
static enum hrtimer_restart mem_timer_function(struct hrtimer *timer)
{
	printk("mem time funcution\n");
	schedule_work(&mem_work); 
	return HRTIMER_NORESTART;
}


/*文件操作结构体*/
static const struct file_operations mem_fops ={
	.owner = THIS_MODULE,//
	.llseek = mem_llseek,
	.read = mem_read,
	.write = mem_write,
	.open = mem_open,
	.release = mem_release,
	.unlocked_ioctl = mem_ioctl,
	//.ioctl = mem_ioctl,
};

 

/*设备驱动模块加载函数*/
static int __init memdev_init(void)
{
	int result;
	int i;
	
	/*利用主设备号、此设备号构造--设备号*/
	 devno = MKDEV(mem_major, 0);//mem_major在.h头文件中为254

	/* 静态申请设备号*/
	if (mem_major)//当mem_major>0，则静态申请设备号
		result = register_chrdev_region(devno, 2, "memdev");
	else  /* mem_major<0,则动态分配设备号 */
	{
		result = alloc_chrdev_region(&devno, 0, 2, "memdev");//动态分配的设备号填到devno
		mem_major = MAJOR(devno);//把主设备号提取出来
	} 

	if (result < 0)
		return result;

	/*初始化cdev结构*/
	cdev_init(&cdev, &mem_fops);//因为cdev定义的是一个静态的结构，所以不需分配空间
	cdev.owner = THIS_MODULE;
	cdev.ops = &mem_fops;

	/* 注册字符设备 */
	cdev_add(&cdev, MKDEV(mem_major, 0), MEMDEV_NR_DEVS);//MEMDEV_NR_DEVS:多少个设备
	
	cdev_class = class_create(THIS_MODULE, "cdevclass");
    if (IS_ERR(cdev_class)) {
        printk("Unable to create class, err = %d\n", (int)PTR_ERR(cdev_class));
        goto fail_malloc;
    }

   cdev_device = device_create(cdev_class, NULL, devno, NULL, "memdev");
    if(NULL ==cdev_device){
        printk("device_create fail\n");
        goto fail_malloc;
    }
   
	
	/* 为设备描述结构分配内存（因为我们用内存模拟设备，操作设备就要操作这段内存）*/
	mem_devp = kmalloc(MEMDEV_NR_DEVS * sizeof(struct mem_dev), GFP_KERNEL);
	if (!mem_devp)    /*申请失败*/
	{
		result =  - ENOMEM;
		goto fail_malloc;
	}

	memset(mem_devp, 0, sizeof(struct mem_dev));
	
	/*为设备分配内存*/
	for (i=0; i < MEMDEV_NR_DEVS; i++){
		mem_devp[i].size = MEMDEV_SIZE;//MEMDEV_SIZE:4096 
		mem_devp[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
		memset(mem_devp[i].data, 0, MEMDEV_SIZE);
	}
	
	result = device_create_file(cdev_device, &dev_attr_reg);
	if(result < 0 ){
		printk("error...device_create_file\n");
		goto fail_create_file;
	}
	result = device_create_file(cdev_device, &dev_attr_debug);
	if(result < 0 )
		goto fail_create_file;

	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ktime = ktime_set(interval/1000000, (interval%1000000)*1000);
	timer.function = mem_timer_function;
//	 hrtimer_start(&timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	 
	hrtimer_start(&timer, ktime, HRTIMER_MODE_REL);
	INIT_WORK(&mem_work, mem_work_func);
	return 0;
	fail_create_file:
	cdev_del(&cdev);   /*注销设备*/
	kfree(mem_devp);     /*释放设备结构体内存*/
	fail_malloc:
	unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*释放设备号*/
	device_destroy(cdev_class, devno);
    class_destroy(cdev_class);
   
	return result;
}


/*模块卸载函数*/
static void __exit memdev_exit(void)
{
	printk("memdev_exit\n");
	cdev_del(&cdev);   /*注销设备*/
	kfree(mem_devp);     /*释放设备结构体内存*/
	unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*释放设备号*/
	device_destroy(cdev_class, devno);
    class_destroy(cdev_class);
}

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("GPL");

module_init(memdev_init);
module_exit(memdev_exit);
