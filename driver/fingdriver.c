#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define FT232R_VENDOR_ID 0x0403
#define FT232R_PRODUCT_ID 0x6001

static struct usb_device_id fingdriver_table[] = {
    { USB_DEVICE(FT232R_VENDOR_ID, FT232R_PRODUCT_ID) },
    { } 
};
MODULE_DEVICE_TABLE(usb, fingdriver_table);

static int fingdriver_probe(struct usb_interface *interface,
                            const struct usb_device_id *id)
{
    struct usb_device *udev = interface_to_usbdev(interface);
    struct usb_device_descriptor *dev_desc = &udev->descriptor;

    printk(KERN_INFO "FT232R device plugged in: Vendor ID=0x%04X, Product ID=0x%04X\n",
           dev_desc->idVendor, dev_desc->idProduct);


    return 0; /* Success */
}

static void fingdriver_disconnect(struct usb_interface *interface)
{
    struct usb_device *udev = interface_to_usbdev(interface);
    struct usb_device_descriptor *dev_desc = &udev->descriptor;

    printk(KERN_INFO "FT232R device unplugged: Vendor ID=0x%04X, Product ID=0x%04X\n",
           dev_desc->idVendor, dev_desc->idProduct);

}

static ssize_t fingdriver_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    
    return len; /* Return the number of bytes written */
}

static ssize_t fingdriver_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    
    return len; /* Return the number of bytes read */
}

static struct usb_driver fingdriver_driver = {
    .name = "fingdriver",
    .id_table = fingdriver_table,
    .probe = fingdriver_probe,
    .disconnect = fingdriver_disconnect,
};

static int fingdriver_major = 47; /* Major number for the driver */

static int __init fingdriver_init(void)
{
    int result;

    /* Register the USB driver */
    result = usb_register(&fingdriver_driver);
    if (result < 0) {
        printk(KERN_ERR "Failed to register FT232R USB driver: %d\n", result);
        return result;
    }

    printk(KERN_INFO "FT232R USB driver registered with major number %d\n", fingdriver_major);
    return 0;
}

static void __exit fingdriver_exit(void)
{
    /* Unregister the USB driver */
    usb_deregister(&fingdriver_driver);

    printk(KERN_INFO "FT232R USB driver unregistered\n");
}

module_init(fingdriver_init);
module_exit(fingdriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("FT232R USB Driver");
