#include <linux/init.h>
#include <linux/module.h>


static int __init hello_init(void)
{
	printk(KERN_INFO "hello world enter\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "hello world exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XU ZHOUBIN <zhoubin.xu@foxmail.com>");
MODULE_DESCRIPTION("A Sample Demo");
MODULE_ALIAS("A Samplest module");
