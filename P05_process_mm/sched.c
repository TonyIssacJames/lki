#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>

#define FIRST_MINOR 0
#define MINOR_CNT 3

struct proc_dir_entry *my_proc_file;
char flag = '0';
static dev_t dev;
static struct cdev c_dev[MINOR_CNT];
static struct class *cl;
static struct task_struct *sleeping_task;
static DECLARE_WAIT_QUEUE_HEAD(wq);  //create the wait queue
static struct task_struct *thread_st[MINOR_CNT] = {NULL};
static char wake_up_code[MINOR_CNT] = {'1'};


static int thread_fn(void *used)
{
	int idx = *(int *)used;
	char ch = wake_up_code[idx];
	printk("Going to sleep thread_f%c\n",ch);
	wait_event_interruptible(wq, flag == ch);
	printk(KERN_INFO "Woken Up thread%c flag = %c\n",ch, flag);
	flag = '0';
	printk("Woken Up\n");
	thread_st[idx] = NULL;
    do_exit(0);
}

int open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "Inside open \n");
	sleeping_task = current;
	return 0;
}

int release(struct inode *inode, struct file *filp) 
{
	printk (KERN_INFO "Inside close \n");
	return 0;
}

ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp) 
{
	int idx = iminor(filp->f_path.dentry->d_inode); //to get minor number from file descriptor

	printk("Inside read \n");
	printk("Scheduling Out\n");
	//TODO 1: Set the task state to TASK_INTERRUPTIBLE
	
	thread_st[idx] = kthread_run(thread_fn, &idx, "mythread%c",wake_up_code[idx]);

	ssleep(10);//sleep for 2 minutes
	printk(KERN_INFO "Main thread is finished\n");
	return 0;
}

ssize_t write(struct file *filp, const char *buff, size_t count, loff_t *offp) 
{   
	return 0;
}

int write_proc(struct file *file,const char *buffer, size_t count, loff_t *off)
{
	int ret = 0;
	printk(KERN_INFO "procfile_write /proc/wait called\n");
	ret = __get_user(flag,buffer);
	printk(KERN_INFO "input is %c\n",flag);

	//TODO 2: Wake up the thread
	if((flag == '1')||(flag == '2')||(flag == '3'))
	{   /*if flag is 1,2, 3 wake up the corresponding thread*/
		printk("Sleeping Process%c will be woken up %c\n",flag, flag);
		//wake_up_process(sleeping_task);
		wake_up_interruptible(&wq); //wakeup one interruptable taks
	}
	else
	{
		printk("All process continues to sleep %c\n",flag);
	}
	return count;
}

struct file_operations p_fops = {
	.write = write_proc
};

struct file_operations pra_fops = {
	read:        read,
	write:        write,
	open:         open,
	release:    release
};


struct cdev *kernel_cdev;

static int create_new_proc_entry(void)
{
	proc_create("wait",0,NULL,&p_fops);
	return 0;
}

int schd_init (void) 
{
	int i, j;
	dev_t my_device;
	
	if (alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "SCD") < 0)
	{
		return -1;
	}
	printk("Major Nr: %d\n", MAJOR(dev));

	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
	{
		unregister_chrdev_region(dev, MINOR_CNT);
		return -1;
	}
	
	for(i=0; i<MINOR_CNT; i++)
	{
		cdev_init(&c_dev[i], &pra_fops);

		my_device = MKDEV(MAJOR(dev), i);
		
		if (cdev_add(&c_dev[i], my_device, 1) == -1)
		{
			class_destroy(cl);
			for(j=i-1; j>=0;j--)
			{
				cdev_del(&c_dev[i]);
			}
			unregister_chrdev_region(dev, MINOR_CNT);
			return -1; 
		}
		
		if (IS_ERR(device_create(cl, NULL, my_device, NULL, "mychar%d", i)))
		{
			class_destroy(cl);
			for(j=i; j>=0;j--)
			{
				cdev_del(&c_dev[i]);
			}
			unregister_chrdev_region(dev, MINOR_CNT);
			return -1;
		}
	}//for(i=0; i<MINOR_CNT; i++)

	create_new_proc_entry();
	
	
	/*static structure init*/
	for(i=0; i<MINOR_CNT; i++)
	{
		wake_up_code[i] = '1' + (char)i; //to set wake_up_code = '1', '2', '3'
	}
	return 0;
}


void schd_cleanup(void) 
{
	int i;
	dev_t my_device;
	
	printk(KERN_INFO " Inside cleanup_module\n");
	remove_proc_entry("wait",NULL);
	
	for(i=0; i<MINOR_CNT; i++)
	{	
		my_device = MKDEV(MAJOR(dev), i);
		cdev_del(&c_dev[i]);
		device_destroy(cl, my_device);
	}
	
	class_destroy(cl);
	unregister_chrdev_region(dev, MINOR_CNT);
	
	printk("Cleaning Up threads\n");
	
	for(i=0; i<MINOR_CNT; i++)
	{
		if (thread_st[i] != NULL)
		{
			kthread_stop(thread_st[i]);
			printk(KERN_INFO "Thread %d stopped\n",i);
		}
	}
}

module_init(schd_init);
module_exit(schd_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysPlay Workshops <workshop@sysplay.in>");
MODULE_DESCRIPTION("Waiting Process Demo");
