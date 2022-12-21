
#include <linux/module.h> // to support modules
#include <linux/kernel.h>
#include <linux/fs.h> // functions on file systems
#include <linux/uaccess.h> // to send data from user to kernel
#include <linux/init.h> // to init module
#include <linux/slab.h> // to map memory
#include <linux/cdev.h> // to create symbol device
#include <linux/ioctl.h>

#include <linux/dcache.h>
#include <linux/fs_struct.h>

static DEFINE_MUTEX(mymutex);

// Define the ioctl code
#define WR_DATA_TASK_STRUCT _IOW('a','a',int32_t*) // magic number, minor number, type
#define WR_DATA_VFSMOUNT _IOW('a','c', int32_t*)
#define RD_DATA _IOR('a','b',int32_t*)


static dev_t first;// identification of first device
static unsigned int count = 1;
static int my_major = 700; // symbol device
static int my_minor = 0; // one type of device
static struct cdev *my_cdev; // include all oper-on which we can do with dev

#define MYDEV_NAME "my_device"
#define KBUF_SIZE (size_t) ((10)*PAGE_SIZE)


static int mychrdev_open(struct inode *inode, struct file *file){

    static int counter = 0;
    char *kbuf = kmalloc(KBUF_SIZE,GFP_KERNEL); // new buffer for all opening file
    if(!kbuf){
        printk(KERN_INFO "Can't allocate memory to kbuf");
        return -1;
    }
    file->private_data = kbuf; // to save addr of buffer
    printk(KERN_INFO "Open device %s \n\n",MYDEV_NAME);
    counter++;
    printk(KERN_INFO "Counter = %d\n",counter);
    printk(KERN_INFO "Module refcounter = %d\n",module_refcount(THIS_MODULE));
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

static int k;
int32_t val = 0;
char buf[256];
static long chr_ioctl(struct file *file, unsigned int cmd,unsigned long arg){
    switch (cmd) {
        case WR_DATA_TASK_STRUCT:
            k = 0;
            copy_from_user(&val, (int32_t *) arg, sizeof(val));
            printk(KERN_INFO "val = %d\n\n", val);
            pid_t pid = (pid_t)val;
            struct task_struct *task_struct = pid_task(find_vpid(pid), PIDTYPE_PID);
            if(task_struct) {
                k += sprintf(buf + k, "task_struct -> pid %d\n", pid);
                k += sprintf(buf + k, "task_struct -> on_cpu %d\n", task_struct->on_cpu);
                k += sprintf(buf + k, "task_struct -> prio %d\n", task_struct->prio);
                k += sprintf(buf + k, "task_struct -> static_prio %d\n", task_struct->static_prio);
                k += sprintf(buf + k, "task_struct -> normal_prio %d\n", task_struct->normal_prio);
                k += sprintf(buf + k, "task_struct -> rt_priority %d\n", task_struct->rt_priority);
            }
            printk(KERN_INFO "Buf -- %s",buf);
            break;
        case WR_DATA_VFSMOUNT:
            k = 0;
            copy_from_user(&val, (int32_t *) arg, sizeof(val));
            printk(KERN_INFO "val = %d\n\n", val);
            pid_t pid_vfm = (pid_t)val;
            struct task_struct *task_struct_vfs = pid_task(find_vpid(pid_vfm), PIDTYPE_PID);
            if(task_struct_vfs) {
                struct fs_struct *fs_struct = task_struct_vfs->fs;
                if(fs_struct){
                    struct path pwd = fs_struct -> pwd;
                        struct vfsmount *vfs = pwd.mnt;
                        if(vfs) {
                            k += sprintf(buf + k, "mnt_flags %d\n", vfs -> mnt_flags);
                            struct dentry *dentry = vfs -> mnt_root;
                            if(dentry) {
                                k += sprintf(buf + k, "d_iname %s\n", dentry -> d_iname);
                                k += sprintf(buf + k, "d_time %ld\n", dentry -> d_time);
                                k += sprintf(buf + k, "d_flags %d\n", dentry -> d_flags);
                                printk(KERN_INFO "Buf -- %s", dentry -> d_iname);
                            }
                        }
                }
            }
            else {
                printk(KERN_INFO "task_struct is null");
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
        .open = mychrdev_open,
        .release = mychrdev_release,
        .unlocked_ioctl = chr_ioctl
};

static int __init init_chrdev(void){
    int ret;
    ret = mutex_trylock(&mymutex);
    if (ret != 0 ) {
        printk(KERN_INFO "mutex is locked\n");
    }
    printk(KERN_INFO "I am loaded"); // sys log, KERN_INFO TYPE OF MSG
    first = MKDEV(my_major, my_minor); // init node to work with dev
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
    if (mutex_is_locked(&mymutex)) {
        printk(KERN_INFO
        "The mutex is locked\n");
        mutex_unlock(&mymutex);
        printk(KERN_INFO "mutex is unlocked\n");
    }
}

module_init(init_chrdev);
module_exit(cleanup_chrdev);

MODULE_LICENSE("GPL");
