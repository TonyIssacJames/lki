#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

static dev_t first; // Global variable for the first device number

static int __init ofcd_init(void) /* Constructor */
{
	int ret;

	printk(KERN_INFO "Namaskar: ofcd registered");
	//TODO 1: Register the char driver with name ofcd with 1 minor
	ret = alloc_chrdev_region(&first,
							  1,
							  1,
							  "ofcd");
	if(ret < 0)
	{
		printk(KERN_INFO "device registartion failed");
		return -1;
	}
	printk(KERN_INFO "<Major, Minor>: <%d, %d>\n", MAJOR(first), MINOR(first));
	return 0;
}

static void __exit ofcd_exit(void) /* Destructor */
{
	//TODO2: //unregister the char driver
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "Alvida: ofcd unregistered");
}

module_init(ofcd_init);
module_exit(ofcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysPlay Workshops <workshop@sysplay.in>");
MODULE_DESCRIPTION("Our First Character Driver");
