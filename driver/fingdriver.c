#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/uaccess.h>

#define FT232R_DEVICE_FILE "/dev/ttyUSB0"

static struct file* fingdriver_file;

static void write_block(const char* block, size_t size)
{
    ssize_t bytes_written = vfs_write(fingdriver_file, block, size, &fingdriver_file->f_pos);

    if (bytes_written < 0) {
        printk(KERN_ERR "Failed to write to fingdriver\n");
    } else if (bytes_written != size) {
        printk(KERN_WARNING "Incomplete write to fingdriver\n");
    }
}

static ssize_t write_char(struct file* file, const char __user* buffer, size_t count, loff_t* offset)
{
    char ch;
    char block[4];

    if (count != 1) {
        return -EINVAL;
    }

    if (copy_from_user(&ch, buffer, sizeof(char)) != 0) {
        return -EFAULT;
    }

    block[0] = ch;
    block[1] = ch;
    block[2] = ch;
    block[3] = ch;

    write_block(block, sizeof(block));

    return sizeof(char);
}

static struct file_operations fingdriver_fops = {
    .owner = THIS_MODULE,
    .write = write_char,
};

static int __init fingdriver_init(void)
{
    fingdriver_file = filp_open(FT232R_DEVICE_FILE, O_WRONLY, 0);
    if (IS_ERR(fingdriver_file)) {
        printk(KERN_ERR "Failed to open fingdriver device file\n");
        return PTR_ERR(fingdriver_file);
    }

    if (!fingdriver_file->f_op->write) {
        filp_close(fingdriver_file, NULL);
        printk(KERN_ERR "fingdriver does not support write operation\n");
        return -EINVAL;
    }

    fingdriver_file->f_op = &fingdriver_fops;

    printk(KERN_INFO "fingdriver initialized!!!!! \n");
    return 0;
}

static void __exit fingdriver_exit(void)
{
    if (fingdriver_file) {
        filp_close(fingdriver_file, NULL);
    }

    printk(KERN_INFO "fingdriver exited\n");
}

module_init(fingdriver_init);
module_exit(fingdriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("fingdriver");
