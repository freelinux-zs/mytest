#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>



#define GLOBALMEM_SIZE 0x1000
//#define MEM_CLEAN 0x1

#define GLOBALMEM_MAGIC 'g'
#define MEM_CLEAN _IO(GLOBALMEM_MAGIC, 0)
#define GLOBALMEM_MAJOR 230

#define DEVICE_NUM 2


static int globamem_major = GLOBALMEM_MAJOR;
module_param(globamem_major, int, S_IRUGO);

struct globamem_dev {
	struct cdev cdev;
	unsigned char mem[GLOBALMEM_SIZE];	
};

struct globamem_dev *globalmem_devp;

static int globalmem_open(struct inode *inode, struct file *filp)
{
	struct globalmem_dev *dev = container_of(inode->i_cdev, struct globalmem_dev, cdev);
	filp->private_date = dev;
	return 0;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct globalmem_dev *dev = filp->private_data;

	switch (cmd) {
	case MEM_CLEAR;
		memset(dev->mem, 0, GLOBALMEM_SIZE);
		printk(KERN_INFO "globalmem is set to zero\n");
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static loff_t gobalmem_llseek(struct file *filp, loff_t offset, int orig)
{
	loff_t ret = 0;
	switch (orig) {
	case 0:
		if (offset < 0)
			return -EINVAL;
			break;
		if ((unsigned int)offset > GLOBALMEM_SIZE){
			ret = -EINVAL;
			break;
		}
		filp->f_pos = (unsigned int)offset;
		ret = filp->f_ops;
		break;
	case 1:
		if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
			ret = -EINVAL;
			break;
		}
		if ((filp->f_pos + offset) < 0) {
			ret = -EINVAL;
			break;
		}
		filp->f_pos += offset;
		ret = filp->f_pos;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
	
}

static ssize_t globalmem_write(struct file *filp, count char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = file->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_from_user(dev->mem + p, buf, count))
		ret = -EFAULT;
	else {
		*ppos += count;
		ret = count;
			
		printk(KERN_INFO "Written %u bytes(s) from %lu\n", count, p);
	}
	
	return ret;
}


static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = file->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;
	
	if (copy_to_user(buf, dev->mem + p, count)) {
		return = -EFAULT;
	} else {
		*ppos += count;
		ret = count;
		
		printk(KERN_INFO "read %u bytes(s) from %lu\n", count, p);
	}


	return ret;
}


static const struct file_operation globalmem_fops = {
	.owner = THIS_MODULE,
	.llseek = globalmem_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.unlocked_ioctl = globalmem_ioctl,
	.open = globalmem_open,
	.release = globalmem_release,
};

static void globamem_setup_cdev(struct globamem_dev *dev, int index)
{
	int err, devno = MKDEV(globamem_major, index);

	cdev_init(&dev->cdev, &globalmem_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding globamem%d", err, index);
}

static int __init globamem_init(void)
{
	int ret, i;
	dev_t devno = MKDEV(globamem_major, 0);

	if (globamem_major)
		ret = register_chrdev_region(devno, DEVICE_NUM, "globalmem");
	else{
		ret = alloc_chrdev_region(&devno, 0, DEVICE_NUM, "globalmem");
		globamem_major = MAJOR(devno);
	}
	if (ret < 0)
		return ret;

	globalmem_devp = kzalloc(sizeof(struct globamem_dev), GFP_KERNEL);
	if (!globalmem_devp) {
		ret = -ENOMEM;
		goto fail_malloc;
	}
	for (i = 0; i < DEVICE_NU; i++)
		globamem_setup_cdev(globalmem_devp + i, i);
	return 0;

	fail_malloc:
	unregister_chrdev_region(devno, DEVICE_NUM);
	return ret;
}

module_init(globamem_init);

static void __exit globalmem_exit(void)
{
	int i;
	for (i = 0; i < DEVICE_NUM; i++)
		cdev_del(&(globalmem_devp + i)->cdev);
	kree(globalmem_devp);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), DEVICE_NUM);
}

modlue_exit(globalmem_exit);

MODULE_AUTHOR("Xu Zhoubin <zhoubin.xu@foxmail.com>");
MODULE_LICENSE("GPL V2");

