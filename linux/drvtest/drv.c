
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include  "drv.h"

static int testdrv_major = 0;
static int testdrv_minor = 0;

static struct class* testdrv_class = NULL;
static struct testdrv_linux_dev* testdrv_dev = NULL;

static int testdrv_open(struct inode* inode, struct file* filp);
static int testdrv_release(struct inode* inode, struct file* filp);
static ssize_t testdrv_read(struct file* flip, char __user* buf, size_t count, loff_t* f_pos);
static ssize_t testdrv_write(struct file* flip, const char __user* buf, size_t count, loff_t* f_pos);


static ssize_t testdrv_proc_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
static ssize_t testdrv_proc_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos);
static int testdrv_proc_open(struct inode *inode, struct file *file);

static struct file_operations testdrv_fops = {
	.owner = THIS_MODULE,
	.open = testdrv_open,
	.read = testdrv_read,
	.write = testdrv_write,
	.release = testdrv_release,
};


static struct file_operations testdrv_proc_fops = {
    //~ .read = testdrv_proc_read,
    //~ .write = testdrv_proc_write,
    //~ .owner = THIS_MODULE,
    	.owner = THIS_MODULE,
		.open = testdrv_proc_open,
		.read = testdrv_proc_read,
		.write = testdrv_proc_write,
		.llseek = seq_lseek,
		.release = single_release,
};


static ssize_t testdrv_val_show(struct device* dev, struct device_attribute* attr, char* buf);
static ssize_t testdrv_val_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count); 

static DEVICE_ATTR(val, S_IRUGO | S_IWUSR, testdrv_val_show, testdrv_val_store);

static int testdrv_open(struct inode* inode, struct file* filp)
{
	struct testdrv_linux_dev* dev;
	dev = container_of(inode->i_cdev, struct testdrv_linux_dev, dev);
	filp->private_data = dev;
	
	
	return 0;
}


static int testdrv_release(struct inode* inode, struct file* filp)
{
	return 0;
}

static ssize_t testdrv_read(struct file* flip, char __user* buf, size_t count, loff_t* f_pos)
{
	ssize_t err = 0;
	struct testdrv_linux_dev* dev = flip->private_data;
	
	if(down_interruptible(&(dev->sem))){
		return -ERESTARTSYS;
	}
	
	if(count < sizeof(dev->val)){
		goto out;
	}
	
	if(copy_to_user(buf, &(dev->val), sizeof(dev->val))){
		err = -EFAULT;
		goto out;
	}
	
	err = sizeof(dev->val);
	
	
out:
	up(&(dev->sem));
	return err;
}


static ssize_t testdrv_write(struct file* flip, const char __user* buf, size_t count, loff_t* f_pos)
{
	struct testdrv_linux_dev* dev = flip->private_data;
	ssize_t err = 0;
	
	if(down_interruptible(&(dev->sem))){
		return  -ERESTARTSYS;
	}
	
	if(count < sizeof(dev->val)){
		goto out;
	}
	
	if(copy_from_user(&(dev->val), buf, count)){
		err = -EFAULT;
		goto out;
	}
	
	err = sizeof(dev->val);
	
out:
	up(&(dev->sem));
	
	
	return err;
}

static ssize_t __testdrv_get_val(struct testdrv_linux_dev* dev, char* buf)
{
	int val = 0;
	
	if(down_interruptible(&(dev->sem))){
		return  -ERESTARTSYS;
	}
	
	val = dev->val;
	up(&(dev->sem));
	
	
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t __testdev_set_val(struct testdrv_linux_dev* dev, const char* buf, size_t count)
{
	int val = 0;
	
	val = simple_strtol(buf, NULL, 10);

	
	if(down_interruptible(&(dev->sem))){
		return -ERESTARTSYS;
	}
	
	dev->val = val;
	up(&(dev->sem));
	
	return count;
}


static ssize_t testdrv_val_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	struct testdrv_linux_dev *hdev = (struct testdrv_linux_dev*)dev_get_drvdata(dev);
	
	return __testdrv_get_val(hdev, buf);
}

static ssize_t testdrv_val_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct testdrv_linux_dev *hdev = (struct testdrv_linux_dev*)dev_get_drvdata(dev);
	
	return __testdev_set_val(hdev, buf, count);
}

static int mux_proc_show(struct seq_file *seq, void *v)
{	
	//~ int MUX_MODE_MAX_LEN = 10;
	//~ int mux_mode = 5;
	//~ char mux_buf[MUX_MODE_MAX_LEN + 1];
	//~ 
	//~ memset(mux_buf, 0, sizeof(mux_buf));
	//~ snprintf(mux_buf, MUX_MODE_MAX_LEN, "%d\n", mux_mode);
	//~ seq_puts(seq, mux_buf);
	//~ return 0;
	
		char* page = NULL;
		page = (char*)__get_free_page(GFP_KERNEL);
		if(!page){
			printk(KERN_ALERT"Failed to malloc page.\n");
			return -ENOMEM;
		}
		
		snprintf(page, PAGE_SIZE, "%d\n", testdrv_dev->val);
		seq_puts(seq, page);
		
		
		free_page((unsigned long)page);
		
		return 0;

}


static int testdrv_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, mux_proc_show, inode->i_private);
}

static ssize_t testdrv_proc_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	return seq_read(file, buf, size, ppos);
}



static ssize_t testdrv_proc_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	int err = 0;
	char* page = NULL;
	
	if(len > PAGE_SIZE){
		printk(KERN_ALERT"the buff is too large: %lu.\n", len);
		return -EFAULT;
	}
	
	page = (char*)__get_free_page(GFP_KERNEL);
	if(!page){
		printk(KERN_ALERT"Failed to malloc page.\n");
		return -ENOMEM;
	}
	
	if(copy_from_user(page, buf, len)){
		printk(KERN_ALERT"Failed to copy buff from user.\n");
		err = -EFAULT;
		goto out;
	}
	
	err = __testdev_set_val(testdrv_dev, page, len);
	
out:
	free_page((unsigned long)page);
	return err;
}

static void testdrv_create_proc(void)
{
	struct proc_dir_entry* proc_file;
//	entry = create_proc_entry(TESTDRV_DEVICES_PROC_NAME, 0, NULL); // old linux
	proc_file = proc_create(TESTDRV_DEVICES_PROC_NAME, 0666,  0, &testdrv_proc_fops);
	if(!proc_file) {
		//~ entry->owner = THIS_MODULE;
		//~ entry->read_proc = testdrv_proc_read;
		//~ entry->write_proc = testdrv_proc_write;
		 printk(KERN_ERR "\nxen:domU Could not create /proc/demo/file");
		 return -ENOMEM;
	}
	
}


static void testdrv_remove_proc(void)
{
	remove_proc_entry(TESTDRV_DEVICES_PROC_NAME, 0);
}


static int __testdrv_setup_drv(struct testdrv_linux_dev* dev)
{
	int err;
	dev_t devno = MKDEV(testdrv_major, testdrv_minor);
	
	memset(dev, 0, sizeof(struct testdrv_linux_dev));
	
	cdev_init(&(dev->dev), &testdrv_fops);
	dev->dev.owner = THIS_MODULE;
	dev->dev.ops = &testdrv_fops;
	
	err = cdev_add(&(dev->dev), devno, 1);
	if(err){
		return err;
	}
	
	sema_init(&(dev->sem), 1);
	dev->val = 0;
	
	return 0;
}


static int __init testdrv_init(void)
{
	int err = -1;
	dev_t dev = 0;
	struct device* temp = NULL;
	
	printk(KERN_ALERT"Initializing testdev devices.\n");
	
	err = alloc_chrdev_region(&dev, 0, 1, TESTDRV_DEVICES_NODE_NAME);
	if(err < 0){
		printk(KERN_ALERT"Failed to alloc char dev region.\n");
		goto fail;
	}
	
	testdrv_major = MAJOR(dev);
	testdrv_minor = MINOR(dev);
	
	testdrv_dev = kmalloc(sizeof(struct testdrv_linux_dev), GFP_KERNEL);
	if(!testdrv_dev){
		err = -ENOMEM;
		printk(KERN_ALERT"Fail to alloc testdrv.\n");
		goto unregister;
	}
	

	err = __testdrv_setup_drv(testdrv_dev);
	if(err){
		printk(KERN_ALERT"Failed to setip dev:%d.\n",err);
		goto cleanup;
	}
	
	testdrv_class = class_create(THIS_MODULE, TESTDRV_DEVICES_CLASS_NAME);
	if(IS_ERR(testdrv_class)){
		err = PTR_ERR(testdrv_class);
		printk(KERN_ALERT"Fail tot create testdrv class.\n");
		goto destroy_cdev;
	}
	
	temp = device_create(testdrv_class, NULL, dev, "%s", TESTDRV_DEVICES_FILE_NAME);
	if(IS_ERR(temp)){
		printk(KERN_ALERT"Failed to creat testdrv devices.\n");
		goto destroy_class;
	}
	
	
	err = device_create_file(temp, &dev_attr_val);
	if(err < 0){
		printk(KERN_ALERT"Failed to create attribute val\n");
		goto destroy_device;
	}
	
	dev_set_drvdata(temp, testdrv_dev);
	
	testdrv_create_proc();
	
	printk(KERN_ALERT"Succeded to initalize hello device.\n");
	
	return 0;
	
destroy_device:
	device_destroy(testdrv_class, dev);

destroy_class:
	class_destroy(testdrv_class);
	
destroy_cdev:
	cdev_del(&(testdrv_dev->dev));
	
cleanup:
	kfree(testdrv_dev);
	
unregister:
	unregister_chrdev_region(MKDEV(testdrv_major, testdrv_minor), 1);
	
fail:
	return err;
}


static void __exit testdrv_exit(void)
{
	dev_t devno = MKDEV(testdrv_major, testdrv_minor);
	printk(KERN_ALERT"Destroy testdrv device.\n");
	testdrv_remove_proc();
	
	if(testdrv_class){
		device_destroy(testdrv_class, MKDEV(testdrv_major,testdrv_minor));
		class_destroy(testdrv_class);
	}
	
	if(testdrv_dev){
		cdev_del(&(testdrv_dev->dev));
		kfree(testdrv_dev);
	}
	
	
	unregister_chrdev_region(devno, 1);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test drv for linux");

module_init(testdrv_init);
module_exit(testdrv_exit);
