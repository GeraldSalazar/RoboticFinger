#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/miscdevice.h>

/* Define estos valores para que coincidan con tus dispositivos */
#define USB_SKEL_VENDOR_ID	0x0403
#define USB_SKEL_PRODUCT_ID	0x6001
/* Tabla de dispositivos compatibles con este controlador */
static struct usb_device_id skel_table [] = {
	{ USB_DEVICE(USB_SKEL_VENDOR_ID, USB_SKEL_PRODUCT_ID) },
	{ }					/* Entrada de terminación */
};
MODULE_DEVICE_TABLE (usb, skel_table);

/* Obtiene un rango menor para tus dispositivos del mantenedor de USB */
#define USB_SKEL_MINOR_BASE	192

/* Estructura para contener toda la información específica del dispositivo */
struct usb_skel {
	struct usb_device *	udev;		 	    /* El dispositivo USB para este dispositivo */
	struct usb_interface *	interface;		/* La interfaz para este dispositivo */
	unsigned char *		bulk_in_buffer;		/* El búfer para recibir datos */
	size_t			bulk_in_size;		    /* El tamaño del búfer de recepción */
	__u8			bulk_in_endpointAddr;	/* La dirección del punto de entrada para datos a granel */
	__u8			bulk_out_endpointAddr;	/* La dirección del punto de salida para datos a granel */
	struct kref		kref;
};
#define to_skel_dev(d) container_of(d, struct usb_skel, kref)

static struct class *fingDriver_class;
static struct device *fingDriver_device;
static struct usb_driver skel_driver;

static void skel_delete(struct kref *kref)
{	
	struct usb_skel *dev = to_skel_dev(kref);

	usb_put_dev(dev->udev);
	kfree (dev->bulk_in_buffer);
	kfree (dev);
}

static int skel_open(struct inode *inode, struct file *file)
{
	struct usb_skel *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);

	interface = usb_find_interface(&skel_driver, subminor);
	if (!interface) {
		pr_err("%s error, no se pudo encontrar el dispositivo para el menor %d",
		     __FUNCTION__, subminor);
		retval = -ENODEV;
		goto exit;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		goto exit;
	}
	
	 /* Incrementa nuestro contador de uso para el dispositivo */
	kref_get(&dev->kref);

	/* Guarda nuestro objeto en la estructura privada del archivo */
	file->private_data = dev;

exit:
	return retval;
}

static int skel_release(struct inode *inode, struct file *file)
{
	struct usb_skel *dev;

	dev = (struct usb_skel *)file->private_data;
	if (dev == NULL)
		return -ENODEV;

	 /* Decrementa el contador en nuestro dispositivo */
	kref_put(&dev->kref, skel_delete);
	return 0;
}

static ssize_t skel_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
	struct usb_skel *dev;
	int retval = 0;

	dev = (struct usb_skel *)file->private_data;
	printk(KERN_INFO "HOLA READ \n");
	/* Realiza una transferencia de lectura a granel desde el dispositivo */
	retval = usb_bulk_msg(dev->udev,
			      usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
			      dev->bulk_in_buffer,
			      min(dev->bulk_in_size, count),
			      (int *) &count, HZ*10);

	/* Se copia la informacional espacio de usuario si se realizo de forma correcta */
	if (!retval) {
		if (copy_to_user(buffer, dev->bulk_in_buffer, count))
			retval = -EFAULT;
		else
			retval = count;
	}

	return retval;
}

static void skel_write_bulk_callback(struct urb *urb)
{
	struct usb_skel *dev = urb->context;
	if (urb->status && 
	    !(urb->status == -ENOENT || 
	      urb->status == -ECONNRESET ||
	      urb->status == -ESHUTDOWN)) {
		dev_dbg(&dev->interface->dev,
			"%s - nonzero write bulk status received: %d",
			__FUNCTION__, urb->status);
	}

	/* Liberar memoria reservada para la transferencia de escritura */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length,
			urb->transfer_buffer, urb->transfer_dma);
}

static ssize_t skel_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos)
{
	struct usb_skel *dev;
	int retval = 0;
	struct urb *urb = NULL;
	char *buf = NULL;

	dev = (struct usb_skel *)file->private_data;

	/* Ver si hay datos que podmeos escribir */
	if (count == 0)
		goto exit;

	/* Reserva memoria para la transferencia de escritura */
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		retval = -ENOMEM;
		goto error;
	}

	buf = usb_alloc_coherent(dev->udev, count, GFP_KERNEL, &urb->transfer_dma);
	if (!buf) {
		retval = -ENOMEM;
		goto error;
	}
	if (copy_from_user(buf, user_buffer, count)) {
		retval = -EFAULT;
		goto error;
	}

	/* Inicializa la estructura urb */
	usb_fill_bulk_urb(urb, dev->udev,
			  usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
			  buf, count, skel_write_bulk_callback, dev);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* Envía la transferencia de escritura */
	retval = usb_submit_urb(urb, GFP_KERNEL);
	if (retval) {
		pr_err("%s - failed submitting write urb, error %d", __FUNCTION__, retval);
		goto error;
	}
    
	printk(KERN_INFO "HOLA WRITE \n");

    /*Hacemos un free de la refernecia del USB*/
	usb_free_urb(urb);

exit:
	return count;

error:
	usb_free_coherent(dev->udev, count, buf, urb->transfer_dma);
	usb_free_urb(urb);
	kfree(buf);
    /* Retornar el número de bytes escritos */
	return retval;
}
/* La struct para le file operator*/
static struct file_operations skel_fops = {
	.owner =	THIS_MODULE,
	.read =		skel_read,
	.write =	skel_write,
	.open =		skel_open,
	.release =	skel_release,
};

/* La strucy para nuesotro usb driver*/
static struct usb_class_driver skel_class = {
	.name = "fingDriver_%d",
	.fops = &skel_fops,
	.minor_base = USB_SKEL_MINOR_BASE,
};

static int skel_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    printk(KERN_INFO "Dispositivo USB conectado\n");
	struct usb_skel *dev = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	size_t buffer_size;
	int i;
	int retval = -ENOMEM;

	/* Asigna memoria para la estructura de datos del dispositivo */
	dev = kzalloc(sizeof(struct usb_skel), GFP_KERNEL);
	if (!dev) {
		pr_err("Out of memory");
		goto error;
	}
	kref_init(&dev->kref);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	//* Configura los puntos de entrada y salida a granel */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (!dev->bulk_in_endpointAddr &&
		    (endpoint->bEndpointAddress & USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* Encontramos un bulk en la salida*/
			buffer_size = endpoint->wMaxPacketSize;
			dev->bulk_in_size = buffer_size;
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			if (!dev->bulk_in_buffer) {
				pr_err("Could not allocate bulk_in_buffer");
				goto error;
			}
		}

		if (!dev->bulk_out_endpointAddr &&
		    !(endpoint->bEndpointAddress & USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* Encontramos un bulk en la salida*/
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
		}
	}
	if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
		pr_err("Could not find both bulk-in and bulk-out endpoints");
		goto error;
	}

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

    // Register a class for the device /
    fingDriver_class = class_create(THIS_MODULE, "fingDriver");
    if (fingDriver_class == NULL) {
      retval = -ENOMEM;
      goto error;
    }

    // Create a device in the class 
    fingDriver_device = device_create(fingDriver_class, NULL, MKDEV(0, 0), NULL, "fingDriver");
    if (fingDriver_device == NULL) {
       retval = -ENOMEM;
       class_destroy(fingDriver_class);
       fingDriver_class = NULL;
       goto error;
    }
	// /* Create a custom attribute file for the device */
    
	//retval = device_create_file(fingDriver_device, &dev_attr_fingdriver);
    //if (retval) {
    //     device_destroy(fingDriver_class, MKDEV(0, 0));
    //     class_destroy(fingDriver_class);
    //     retval = -EINVAL;
    //     goto error;
    // }

	/* Registra el dispositivo como una clase USB */
	retval = usb_register_dev(interface, &skel_class);
	if (retval) {
		/* something prevented us from registering this driver */
		pr_err("Not able to get a minor for this device.");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/* Incrementa nuestro contador de uso para el dispositivo */
	dev_info(&interface->dev, "USB Skeleton device now attached to USBSkel-%d", interface->minor);
	return 0;

error:
	if (dev)
		kref_put(&dev->kref, skel_delete);
	return retval;
}

static void skel_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "Dispositivo USB desconectado\n");
	struct usb_skel *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* Desconecta el dispositivo de la clase USB */
	usb_deregister_dev(interface, &skel_class);

	/* Decrementa el contador en nuestro dispositivo */
	kref_put(&dev->kref, skel_delete);
	// Destroy the device
    device_destroy(fingDriver_class, MKDEV(0, 0));
    // Destroy the class 
    class_destroy(fingDriver_class); 
    
	dev_info(&interface->dev, "USB Skeleton #%d now disconnected", minor);
}

static struct usb_driver skel_driver = {
	.name = "test_driver",
	.id_table = skel_table,
	.probe = skel_probe,
	.disconnect = skel_disconnect,
};

static int __init usb_skel_init(void)
{
	int result;
	printk(KERN_INFO "HOLA MUNDO \n");

	/* Registra el controlador USB con el núcleo */
	result = usb_register(&skel_driver);
	if (result)
		pr_err("usb_register failed. Error number %d", result);

	return result;
}

static void __exit usb_skel_exit(void)
{
	printk(KERN_INFO "ADIOS HOLA MUNDO \n");
	/* Desregistra el controlador USB del núcleo */
	usb_deregister(&skel_driver);
}

module_init (usb_skel_init);
module_exit (usb_skel_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aldo");
MODULE_DESCRIPTION("FT232R USB Driver");