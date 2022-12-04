//
// Created by dinar on 04.12.2022.
//
#include <linux/module.h> // to support modules
#include <linux/kernel.h>
#include <linux/fs.h> // functions on file systems
#include <linux/uaccess.h> // to send data from user to kernel
#include <linux/init.h> // to init module
#include <linux/slab.h> // to map memory
#include <linux/cdev.h> // to create symbol device
#include <linux/ioctl.h>

#include <linux/netdevice.h>
#include <linux/device-mapper.h>
#include <linux/sched/signal.h>
#include <linux/tty.h>
#include <linux/kdev_t.h>

// Define the ioctl code
#define WR_DATA_NET_DEVICE _IOW('a','a',int32_t*) // magic number, minor number, type
#define WR_DATA_DM_IO_MEMORY _IOW('a','c', int32_t*)
#define RD_DATA _IOR('a','b',int32_t*)


static dev_t first;// identification of first device
static unsigned int count = 1;
static int my_major = 700; // symbol device
static int my_minor = 0; // one type of device
static struct cdev *my_cdev;// include all oper-on which we can do with dev

#define MYDEV_NAME "my_device"
#define KBUF_SIZE (size_t) ((10)*PAGE_SIZE)


static int mychrdev_open(struct inode *inode, struct file *file){
    static int counter = 0;
    char *kbuf = kmalloc(KBUF_SIZE,GFP_KERNEL); // new buffer for all opening file
    if(!kbuf){
        printk(KERN_INFO "Can't allocate memory to kbuf!!!");
        return -1;
    }
    file->private_data = kbuf; // to save addr of buffer
    printk(KERN_INFO "Open device %s \n\n",MYDEV_NAME);
    counter++;
    printk(KERN_INFO "Counter  = %d\n",counter);
    printk(KERN_INFO "Module refcounter  = %d\n",module_refcount(THIS_MODULE));
    return 0;
}

static int mychrdev_release(struct inode *inode, struct file *file){
    printk(KERN_INFO "Release device %s \n\n",MYDEV_NAME);
    char *kbuf = file->private_data;
    printk(KERN_INFO "Free buffer");
    if(kbuf){
        kfree(kbuf);
    }
    kbuf = NULL;
    file->private_data = NULL;
    return 0;
}

//static ssize_t mychrdev_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos){
//char *kbuf = file->private_data;
//int nbytes = lbuf - copy_to_user(buf, kbuf + *ppos, lbuf); // count how many bytes we have
//*ppos += nbytes;
//printk(KERN_INFO "Read device %s nbytes = %d, ppos = %d; \n\n",MYDEV_NAME, nbytes, (int)*ppos);
//return nbytes;
//}
//
//static ssize_t mychrdev_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos){
//char *kbuf = file->private_data;
//int nbytes = lbuf - copy_from_user(kbuf + *ppos, buf, lbuf); // copy from user check validness
//*ppos +=nbytes;
//printk(KERN_INFO "Write device %s nbytes = %d, ppos = %d \n\n",MYDEV_NAME, nbytes,(int)*ppos);
//return nbytes;
//}

static int k;
int32_t val = 0;
char buf[256];
static long chr_ioctl(struct file *file, unsigned int cmd,unsigned long arg){
    switch (cmd) {
        case WR_DATA_NET_DEVICE:
            k = 0;
            copy_from_user(&val, (int32_t *) arg, sizeof(val));
            printk(KERN_INFO "val = %d\n\n", val);
            pid_t pid = (pid_t)val;
            struct task_struct *task_struct = pid_task(find_vpid(pid), PIDTYPE_PID);
            if(task_struct) {
                k += sprintf(buf + k, "task_struct -> pid %d\n", pid);
                k += sprintf(buf + k, "task_struct -> on_cpu %d\n", task_struct->on_cpu);
            }
            printk(KERN_INFO "Buf -- %s",buf);
            break;
        case WR_DATA_DM_IO_MEMORY:
            k = 0;
            copy_from_user(&val, (int32_t *) arg, sizeof(val));
            printk(KERN_INFO "val = %d\n\n", val);
            pid_t pid = (pid_t)val;
            struct task_struct *task_struct = pid_task(find_vpid(pid), PIDTYPE_PID);
            if(task_struct) {
                k += sprintf(buf + k, "task_struct -> pid %d\n", pid);
                k += sprintf(buf + k, "task_struct -> on_cpu %d\n", task_struct->on_cpu);
            }
            printk(KERN_INFO "Buf -- %s",buf);
            break;
        case RD_DATA:
            copy_to_user(arg, buf,k);
            break;
    }
    return 0;
}

static const struct file_operations my_cdev_fops = {
        .owner = THIS_MODULE,
//        .read = mychrdev_read,
//        .write = mychrdev_write,
        .open = mychrdev_open,
        .release = mychrdev_release,
        .unlocked_ioctl = chr_ioctl
};

static int __init init_chrdev(void){
    printk(KERN_INFO "I am loaded");// sys log, KERN_INFO TYPE OF MSG
    first = MKDEV(my_major, my_minor);// init node to work with dev
    register_chrdev_region(first, count, MYDEV_NAME);// to reg region for dev examples
    my_cdev =  cdev_alloc(); // map memory for struct cdev
    cdev_init(my_cdev, &my_cdev_fops); // init fops
    cdev_add(my_cdev,first,count); // add devs (one device)
    return 0;
}

static void __exit cleanup_chrdev(void){
    printk(KERN_INFO "I am unloaded");
    if (my_cdev){
        cdev_del(my_cdev);
        unregister_chrdev_region(first,count);
    }
}

module_init(init_chrdev);
module_exit(cleanup_chrdev);

MODULE_LICENSE("GPL");//