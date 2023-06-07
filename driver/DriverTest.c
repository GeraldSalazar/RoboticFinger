#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/serial.h>
#include <linux/gpio/driver.h>
#include <linux/usb/serial.h>
#include "DriverTest.h"
#include "DriverTestID.h"

struct ftdi_private {
	enum ftdi_chip_type chip_type; /* type of device, either SIO or FT8U232AM */
	int baud_base;		/* baud base clock for divisor setting */
	int custom_divisor;	/* custom_divisor kludge, this is for baud_base (different from what goes to the chip!) */
	u16 last_set_data_value; /* the last data state set - needed for doing a break*/
	int flags;		/* some ASYNC_xxxx flags are supported */
	unsigned long last_dtr_rts;	/* saved modem control outputs */
	char prev_status;        /* Used for TIOCMIWAIT */
	char transmit_empty;	/* If transmitter is empty or not */
	u16 interface;		/* FT2232C, FT2232H or FT4232H port interface (0 for FT232/245) */
	speed_t force_baud;	/* if non-zero, force the baud rate tothis value */
	int force_rtscts;	/* if non-zero, force RTS-CTS to alwaysbe enabled */

	unsigned int latency;		/* latency setting in use */
	unsigned short max_packet_size;
	struct mutex cfg_lock; /* Avoid mess by parallel calls of config ioctl() and change_speed() */

#ifdef CONFIG_GPIOLIB
	struct gpio_chip gc;
	struct mutex gpio_lock;	/* protects GPIO state */
	bool gpio_registered;	/* is the gpiochip in kernel registered */
	bool gpio_used;		    /* true if the user requested a gpio */
	u8 gpio_altfunc;	    /* which pins are in gpio mode */
	u8 gpio_output;		    /* pin directions cache */
	u8 gpio_value;		    /* pin value for outputs */
#endif
};

/* struct ftdi_sio_quirk is used by devices requiring special attention. */
struct ftdi_sio_quirk {
	int (*probe)(struct usb_serial *);
	/* Special settings for probed ports. */
	void (*port_probe)(struct ftdi_private *);
};

static const struct usb_device_id id_table_combined[] = {
	{ USB_DEVICE(FTDI_VID, FTDI_8U232AM_PID) },
	{ }					/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, id_table_combined);

static const char *ftdi_chip_name[] = {
	[FT232RL] = "FT232RL",
};

/* function prototypes for a FTDI serial converter */
static int  ftdi_sio_probe(struct usb_serial *serial,const struct usb_device_id *id);
static int  ftdi_sio_port_probe(struct usb_serial_port *port);
static void ftdi_sio_port_remove(struct usb_serial_port *port);
static int  ftdi_open(struct tty_struct *tty, struct usb_serial_port *port);
static int ftdi_prepare_write_buffer(struct usb_serial_port *port,void *dest, size_t size);

static struct usb_serial_driver ftdi_sio_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"DriverTest",
	},
	.description =		"USB Seria Test - Aldo",
	.id_table =		id_table_combined,
	.num_ports =		1,
	.probe =		ftdi_sio_probe,
	.port_probe =		ftdi_sio_port_probe,
	.port_remove =		ftdi_sio_port_remove,
	.open =			ftdi_open,
	.prepare_write_buffer =	ftdi_prepare_write_buffer,
};

static struct usb_serial_driver * const serial_drivers[] = {
	&ftdi_sio_device, NULL
};

#define WDR_TIMEOUT 5000 /* default urb timeout */

// Funciones de utilidad
static int write_latency_timer(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_device *udev = port->serial->dev;
	int rv;
	int l = priv->latency;

	if (priv->chip_type == SIO || priv->chip_type == FT8U232AM)
		return -EINVAL;

	if (priv->flags & ASYNC_LOW_LATENCY)
		l = 1;

	dev_dbg(&port->dev, "%s: setting latency timer = %i\n", __func__, l);

	rv = usb_control_msg(udev,
			     usb_sndctrlpipe(udev, 0),
			     FTDI_SIO_SET_LATENCY_TIMER_REQUEST,
			     FTDI_SIO_SET_LATENCY_TIMER_REQUEST_TYPE,
			     l, priv->interface,
			     NULL, 0, WDR_TIMEOUT);
	if (rv < 0)
		dev_err(&port->dev, "Unable to write latency timer: %i\n", rv);
	return rv;
}

static int _read_latency_timer(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_device *udev = port->serial->dev;
	u8 buf;
	int rv;

	rv = usb_control_msg_recv(udev, 0, FTDI_SIO_GET_LATENCY_TIMER_REQUEST,
				  FTDI_SIO_GET_LATENCY_TIMER_REQUEST_TYPE, 0,
				  priv->interface, &buf, 1, WDR_TIMEOUT,
				  GFP_KERNEL);
	if (rv == 0)
		rv = buf;

	return rv;
}

static int read_latency_timer(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	int rv;

	if (priv->chip_type == SIO || priv->chip_type == FT8U232AM)
		return -EINVAL;

	rv = _read_latency_timer(port);
	if (rv < 0) {
		dev_err(&port->dev, "Unable to read latency timer: %i\n", rv);
		return rv;
	}

	priv->latency = rv;

	return 0;
}
/* Determine type of FTDI chip based on USB config and descriptor. */
static void ftdi_determine_type(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;
	struct usb_device *udev = serial->dev;
	unsigned version;
	unsigned interfaces;

	/* Assume it is not the original SIO device for now. */
	priv->baud_base = 48000000 / 2;

	version = le16_to_cpu(udev->descriptor.bcdDevice);
	interfaces = udev->actconfig->desc.bNumInterfaces;
	dev_dbg(&port->dev, "%s: bcdDevice = 0x%x, bNumInterfaces = %u\n", __func__,
		version, interfaces);
	if (version < 0x900) {
		/* Assume it's an FT232RL */
		priv->chip_type = FT232RL;
	} 

	dev_info(&udev->dev, "Detected %s\n", ftdi_chip_name[priv->chip_type]);
}

/*
 * Determine the maximum packet size for the device. This depends on the chip
 * type and the USB host capabilities. The value should be obtained from the
 * device descriptor as the chip will use the appropriate values for the host.
 */
static void ftdi_set_max_packet_size(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_interface *interface = port->serial->interface;
	struct usb_endpoint_descriptor *ep_desc;
	unsigned num_endpoints;
	unsigned i;

	num_endpoints = interface->cur_altsetting->desc.bNumEndpoints;
	if (!num_endpoints)
		return;

	for (i = 0; i < num_endpoints; i++) {
		ep_desc = &interface->cur_altsetting->endpoint[i].desc;
		if (!ep_desc->wMaxPacketSize) {
			ep_desc->wMaxPacketSize = cpu_to_le16(0x40);
			dev_warn(&port->dev, "Overriding wMaxPacketSize on endpoint %d\n",
					usb_endpoint_num(ep_desc));
		}
	}
	priv->max_packet_size = usb_endpoint_maxp(ep_desc);
}

//
// Atributos del Sysfs
//
static ssize_t latency_timer_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct usb_serial_port *port = to_usb_serial_port(dev);
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	if (priv->flags & ASYNC_LOW_LATENCY)
		return sprintf(buf, "1\n");
	else
		return sprintf(buf, "%u\n", priv->latency);
}

/* Write a new value of the latency timer, in units of milliseconds. */
static ssize_t latency_timer_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *valbuf, size_t count)
{
	struct usb_serial_port *port = to_usb_serial_port(dev);
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	u8 v;
	int rv;

	if (kstrtou8(valbuf, 10, &v))
		return -EINVAL;

	priv->latency = v;
	rv = write_latency_timer(port);
	if (rv < 0)
		return -EIO;
	return count;
}
static DEVICE_ATTR_RW(latency_timer);

/* Write an event character directly to the FTDI register.  The ASCII
   value is in the low 8 bits, with the enable bit in the 9th bit. */
static ssize_t event_char_store(struct device *dev,
	struct device_attribute *attr, const char *valbuf, size_t count)
{
	struct usb_serial_port *port = to_usb_serial_port(dev);
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_device *udev = port->serial->dev;
	unsigned int v;
	int rv;

	if (kstrtouint(valbuf, 0, &v) || v >= 0x200)
		return -EINVAL;

	dev_dbg(&port->dev, "%s: setting event char = 0x%03x\n", __func__, v);

	rv = usb_control_msg(udev,
			     usb_sndctrlpipe(udev, 0),
			     FTDI_SIO_SET_EVENT_CHAR_REQUEST,
			     FTDI_SIO_SET_EVENT_CHAR_REQUEST_TYPE,
			     v, priv->interface,
			     NULL, 0, WDR_TIMEOUT);
	if (rv < 0) {
		dev_dbg(&port->dev, "Unable to write event character: %i\n", rv);
		return -EIO;
	}

	return count;
}
static DEVICE_ATTR_WO(event_char);

static int create_sysfs_attrs(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	int retval = 0;
	if (priv->chip_type != SIO) {
		dev_dbg(&port->dev, "sysfs attributes for %s\n", ftdi_chip_name[priv->chip_type]);
		retval = device_create_file(&port->dev, &dev_attr_event_char);
		if ((!retval) &&
		    (priv->chip_type == FT232RL)) {
			retval = device_create_file(&port->dev,
						    &dev_attr_latency_timer);
		}
	}
	return retval;
}

static void remove_sysfs_attrs(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	if (priv->chip_type != SIO) {
		device_remove_file(&port->dev, &dev_attr_event_char);
		if (priv->chip_type == FT232RL) {
			device_remove_file(&port->dev, &dev_attr_latency_timer);
		}
	}

}

#ifdef CONFIG_GPIOLIB

static int ftdi_set_bitmode(struct usb_serial_port *port, u8 mode)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;
	int result;
	u16 val;

	result = usb_autopm_get_interface(serial->interface);
	if (result)
		return result;

	val = (mode << 8) | (priv->gpio_output << 4) | priv->gpio_value;
	result = usb_control_msg(serial->dev,
				 usb_sndctrlpipe(serial->dev, 0),
				 FTDI_SIO_SET_BITMODE_REQUEST,
				 FTDI_SIO_SET_BITMODE_REQUEST_TYPE, val,
				 priv->interface, NULL, 0, WDR_TIMEOUT);
	if (result < 0) {
		dev_err(&serial->interface->dev,
			"bitmode request failed for value 0x%04x: %d\n",
			val, result);
	}

	usb_autopm_put_interface(serial->interface);

	return result;
}

static int ftdi_set_cbus_pins(struct usb_serial_port *port)
{
	return ftdi_set_bitmode(port, FTDI_SIO_BITMODE_CBUS);
}

static int ftdi_exit_cbus_mode(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);

	priv->gpio_output = 0;
	priv->gpio_value = 0;
	return ftdi_set_bitmode(port, FTDI_SIO_BITMODE_RESET);
}

static int ftdi_gpio_request(struct gpio_chip *gc, unsigned int offset)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	int result;

	mutex_lock(&priv->gpio_lock);
	if (!priv->gpio_used) {
		/* Set default pin states, as we cannot get them from device */
		priv->gpio_output = 0x00;
		priv->gpio_value = 0x00;
		result = ftdi_set_cbus_pins(port);
		if (result) {
			mutex_unlock(&priv->gpio_lock);
			return result;
		}

		priv->gpio_used = true;
	}
	mutex_unlock(&priv->gpio_lock);

	return 0;
}

static int ftdi_read_cbus_pins(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;
	u8 buf;
	int result;

	result = usb_autopm_get_interface(serial->interface);
	if (result)
		return result;

	result = usb_control_msg_recv(serial->dev, 0,
				      FTDI_SIO_READ_PINS_REQUEST,
				      FTDI_SIO_READ_PINS_REQUEST_TYPE, 0,
				      priv->interface, &buf, 1, WDR_TIMEOUT,
				      GFP_KERNEL);
	if (result == 0)
		result = buf;

	usb_autopm_put_interface(serial->interface);

	return result;
}

static int ftdi_gpio_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	int result;

	result = ftdi_read_cbus_pins(port);
	if (result < 0)
		return result;

	return !!(result & BIT(gpio));
}

static void ftdi_gpio_set(struct gpio_chip *gc, unsigned int gpio, int value)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	struct ftdi_private *priv = usb_get_serial_port_data(port);

	mutex_lock(&priv->gpio_lock);

	if (value)
		priv->gpio_value |= BIT(gpio);
	else
		priv->gpio_value &= ~BIT(gpio);

	ftdi_set_cbus_pins(port);

	mutex_unlock(&priv->gpio_lock);
}

static int ftdi_gpio_get_multiple(struct gpio_chip *gc, unsigned long *mask,
					unsigned long *bits)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	int result;

	result = ftdi_read_cbus_pins(port);
	if (result < 0)
		return result;

	*bits = result & *mask;

	return 0;
}

static void ftdi_gpio_set_multiple(struct gpio_chip *gc, unsigned long *mask,
					unsigned long *bits)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	struct ftdi_private *priv = usb_get_serial_port_data(port);

	mutex_lock(&priv->gpio_lock);

	priv->gpio_value &= ~(*mask);
	priv->gpio_value |= *bits & *mask;
	ftdi_set_cbus_pins(port);

	mutex_unlock(&priv->gpio_lock);
}

static int ftdi_gpio_direction_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	struct ftdi_private *priv = usb_get_serial_port_data(port);

	return !(priv->gpio_output & BIT(gpio));
}

static int ftdi_gpio_direction_input(struct gpio_chip *gc, unsigned int gpio)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	int result;

	mutex_lock(&priv->gpio_lock);

	priv->gpio_output &= ~BIT(gpio);
	result = ftdi_set_cbus_pins(port);

	mutex_unlock(&priv->gpio_lock);

	return result;
}

static int ftdi_gpio_direction_output(struct gpio_chip *gc, unsigned int gpio,
					int value)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	int result;

	mutex_lock(&priv->gpio_lock);

	priv->gpio_output |= BIT(gpio);
	if (value)
		priv->gpio_value |= BIT(gpio);
	else
		priv->gpio_value &= ~BIT(gpio);

	result = ftdi_set_cbus_pins(port);

	mutex_unlock(&priv->gpio_lock);

	return result;
}

static int ftdi_gpio_init_valid_mask(struct gpio_chip *gc,
				     unsigned long *valid_mask,
				     unsigned int ngpios)
{
	struct usb_serial_port *port = gpiochip_get_data(gc);
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	unsigned long map = priv->gpio_altfunc;

	bitmap_complement(valid_mask, &map, ngpios);

	if (bitmap_empty(valid_mask, ngpios))
		dev_dbg(&port->dev, "no CBUS pin configured for GPIO\n");
	else
		dev_dbg(&port->dev, "CBUS%*pbl configured for GPIO\n", ngpios,
			valid_mask);

	return 0;
}

static int ftdi_read_eeprom(struct usb_serial *serial, void *dst, u16 addr,
				u16 nbytes)
{
	int read = 0;

	if (addr % 2 != 0)
		return -EINVAL;
	if (nbytes % 2 != 0)
		return -EINVAL;
	while (read < nbytes) {
		int rv;

		rv = usb_control_msg(serial->dev,
				     usb_rcvctrlpipe(serial->dev, 0),
				     FTDI_SIO_READ_EEPROM_REQUEST,
				     FTDI_SIO_READ_EEPROM_REQUEST_TYPE,
				     0, (addr + read) / 2, dst + read, 2,
				     WDR_TIMEOUT);
		if (rv < 2) {
			if (rv >= 0)
				return -EIO;
			else
				return rv;
		}

		read += rv;
	}

	return 0;
}

static int ftdi_gpio_init_ft232r(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	u16 cbus_config;
	u8 *buf;
	int ret;
	int i;

	buf = kmalloc(2, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = ftdi_read_eeprom(port->serial, buf, 0x14, 2);
	if (ret < 0)
		goto out_free;

	cbus_config = le16_to_cpup((__le16 *)buf);
	dev_dbg(&port->dev, "cbus_config = 0x%04x\n", cbus_config);

	priv->gc.ngpio = 4;

	priv->gpio_altfunc = 0xff;
	for (i = 0; i < priv->gc.ngpio; ++i) {
		if ((cbus_config & 0xf) == FTDI_FT232R_CBUS_MUX_GPIO)
			priv->gpio_altfunc &= ~BIT(i);
		cbus_config >>= 4;
	}
out_free:
	kfree(buf);

	return ret;
}

static int ftdi_gpio_init(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;
	int result;
	result = ftdi_gpio_init_ft232r(port);

	if (result < 0)
		return result;

	mutex_init(&priv->gpio_lock);

	priv->gc.label = "ftdi-cbus";
	priv->gc.request = ftdi_gpio_request;
	priv->gc.get_direction = ftdi_gpio_direction_get;
	priv->gc.direction_input = ftdi_gpio_direction_input;
	priv->gc.direction_output = ftdi_gpio_direction_output;
	priv->gc.init_valid_mask = ftdi_gpio_init_valid_mask;
	priv->gc.get = ftdi_gpio_get;
	priv->gc.set = ftdi_gpio_set;
	priv->gc.get_multiple = ftdi_gpio_get_multiple;
	priv->gc.set_multiple = ftdi_gpio_set_multiple;
	priv->gc.owner = THIS_MODULE;
	priv->gc.parent = &serial->interface->dev;
	priv->gc.base = -1;
	priv->gc.can_sleep = true;

	result = gpiochip_add_data(&priv->gc, port);
	if (!result)
		priv->gpio_registered = true;

	return result;
}

static void ftdi_gpio_remove(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);

	if (priv->gpio_registered) {
		gpiochip_remove(&priv->gc);
		priv->gpio_registered = false;
	}

	if (priv->gpio_used) {
		/* Exiting CBUS-mode does not reset pin states. */
		ftdi_exit_cbus_mode(port);
		priv->gpio_used = false;
	}
}

#else

static int ftdi_gpio_init(struct usb_serial_port *port)
{
	return 0;
}

static void ftdi_gpio_remove(struct usb_serial_port *port) { }

#endif	/* CONFIG_GPIOLIB */

//
// Funciones del file operatior
//
static int ftdi_sio_probe(struct usb_serial *serial,const struct usb_device_id *id)
{
	const struct ftdi_sio_quirk *quirk =(struct ftdi_sio_quirk *)id->driver_info;

	if (quirk && quirk->probe) {
		int ret = quirk->probe(serial);
		if (ret != 0)
			return ret;
	}

	usb_set_serial_data(serial, (void *)id->driver_info);

	return 0;
}

static int ftdi_sio_port_probe(struct usb_serial_port *port)
{
	struct ftdi_private *priv;
	const struct ftdi_sio_quirk *quirk = usb_get_serial_data(port->serial);
	int result;

	priv = kzalloc(sizeof(struct ftdi_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	mutex_init(&priv->cfg_lock);

	if (quirk && quirk->port_probe)
		quirk->port_probe(priv);

	usb_set_serial_port_data(port, priv);

	ftdi_determine_type(port);
	ftdi_set_max_packet_size(port);
	if (read_latency_timer(port) < 0)
		priv->latency = 16;
	write_latency_timer(port);
	create_sysfs_attrs(port);

	result = ftdi_gpio_init(port);
	if (result < 0) {
		dev_err(&port->serial->interface->dev,
			"GPIO initialisation failed: %d\n",
			result);
	}

	return 0;
}

static void ftdi_sio_port_remove(struct usb_serial_port *port)
{
	struct ftdi_private *priv = usb_get_serial_port_data(port);

	ftdi_gpio_remove(port);

	remove_sysfs_attrs(port);

	kfree(priv);
}

static int ftdi_open(struct tty_struct *tty, struct usb_serial_port *port)
{
	struct usb_device *dev = port->serial->dev;
	struct ftdi_private *priv = usb_get_serial_port_data(port);
	usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			FTDI_SIO_RESET_REQUEST, FTDI_SIO_RESET_REQUEST_TYPE,
			FTDI_SIO_RESET_SIO,
			priv->interface, NULL, 0, WDR_TIMEOUT);

	return usb_serial_generic_open(tty, port);
}

static int ftdi_prepare_write_buffer(struct usb_serial_port *port,void *dest, size_t size){
	struct ftdi_private *priv;
	int count;
	unsigned long flags;

	priv = usb_get_serial_port_data(port);

	if (priv->chip_type == SIO) {
		unsigned char *buffer = dest;
		int i, len, c;
		count = 0;
		spin_lock_irqsave(&port->lock, flags);
		for (i = 0; i < size - 1; i += priv->max_packet_size) {
			len = min_t(int, size - i, priv->max_packet_size) - 1;
			c = kfifo_out(&port->write_fifo, &buffer[i + 1], len);
			if (!c)
				break;
			port->icount.tx += c;
			buffer[i] = (c << 2) + 1;
			count += c + 1;
		}
		spin_unlock_irqrestore(&port->lock, flags);
	} else {
		count = kfifo_out_locked(&port->write_fifo, dest, size,
								&port->lock);
		port->icount.tx += count;
	}
	return count;
}
module_usb_serial_driver(serial_drivers, id_table_combined);
MODULE_AUTHOR("Aldo");
MODULE_DESCRIPTION("USB - Serial Driver SO");
MODULE_LICENSE("GPL");