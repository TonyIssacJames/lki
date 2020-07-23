#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#define FIRST_MINOR 0
#define MINOR_CNT 1

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: In open\n");
	return 0;
}
static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: In close\n");
	return 0;
}

static char c = 'A';

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Driver: In read - Buf: %p; Len: %zd; Off: %Ld\nData:\n", buf, len, *off);
    return 0; //just for testing remove it
	if (*off == 0)
	{
		if (copy_to_user(buf, &c, 1))
		{
			return -EFAULT;
		}
		*off += 1;
		return 1;
	}
	else
		return 0;
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Driver: In write - Buf: %p; Len: %zd; Off: %Ld\nData:\n", buf, len, *off);
    return 0; //just for testing remove it
	if (copy_from_user(&c, buf + len - 1, 1))
	{
		return -EFAULT;
	}
	return len;
}

static struct file_operations driver_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

static int __init fcd_init(void)
{
	int ret;
	struct device *dev_ret;

	if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "final_driver_Tony")) < 0)
	{
		printk(KERN_INFO "device registartion failed");
		return ret;
	}

	printk(KERN_INFO "(Major, Minor): (%d, %d)\n", MAJOR(dev), MINOR(dev));
	cdev_init(&c_dev, &driver_fops);

	if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
	{
		printk(KERN_INFO "cdev_add failed");
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return ret;
	}

	//TODO 1: Create the class with name char & assign it to cl.
	//Refer include/linux/device.h for class_create() prototype
	cl = class_create(THIS_MODULE, "chardrv_Tony");
	
	if (IS_ERR(cl))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(cl);
	}
	//TODO 2: Create the device & assign it to dev_ret
	//Refer include/linux/device.h for device_create() prototype. 2nd & 4th arguments are NULL
	dev_ret = device_create(cl, NULL, dev, NULL, "mynull_Tony");
		
	if (IS_ERR(dev_ret))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(dev_ret);
	}

    printk(KERN_INFO "Success in fcd_init\n");
	return 0;
}

static void __exit fcd_exit(void)
{
	//TODO 3: Delete the device
	//TODO 4: Delete the class
	device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MINOR_CNT);
    printk(KERN_INFO "Success in fcd_exit\n");
}

module_init(fcd_init);
module_exit(fcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysPlay Workshops <workshop@sysplay.in>");
MODULE_DESCRIPTION("Final Character Driver Template");
