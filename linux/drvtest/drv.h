#ifndef __DRV_TEST_H__
#define __DRV_TEST_H__
#include <linux/cdev.h>
#include <linux/semaphore.h>

#define TESTDRV_DEVICES_NODE_NAME "testdrv"
#define TESTDRV_DEVICES_FILE_NAME "testdrvfile"
#define TESTDRV_DEVICES_PROC_NAME "testdrvporc"
#define TESTDRV_DEVICES_CLASS_NAME "testdrvclass"

struct testdrv_linux_dev {
	int val;
	struct semaphore sem;
	struct cdev dev;
};

#endif
