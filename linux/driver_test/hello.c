#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>


static unsigned int variable;
static struct proc_dir_entry *test_dir, *test_entry;


static int test_proc_show(struct seq_file *seq, void *v)
{
	unsigned int *ptr_val = seq->private;
	seq_printf(seq, "%u\n", *ptr_val);
	return 0;
}

static ssize_t test_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	struct seq_file *seq = file->private_data;
	unsigned int *ptr_val = seq->private;
	
	*ptr_val = simple_strtoul(buffer, NULL, 10);
	return count;
}

static int test_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, test_proc_show, PDE_DATA(inode));
}

static const struct file_operations test_proc_fops = {
	.owner = THIS_MODULE,
	.open = test_proc_open,
	.read = seq_read,
	.write = test_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init hello_init(void)
{
	printk(KERN_INFO "hello world enter\n");

	test_dir = proc_mkdir("test_dir", NULL);
	if (test_dir){
		test_entry = proc_create_data("test_rw", 0666, test_dir, &test_proc_fops, &variable);
		if (test_entry)
			return 0;
	}

	return -ENOMEM;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "hello world exit\n");
	remove_proc_entry("test_rw", test_dir);
	remove_proc_entry("test_dir", NULL);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XU ZHOUBIN <zhoubin.xu@foxmail.com>");
MODULE_DESCRIPTION("A Sample Demo");
MODULE_ALIAS("A Samplest module");
