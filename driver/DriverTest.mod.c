#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x6383b27c, "__x86_indirect_thunk_rdx" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0x13d0adf7, "__kfifo_out" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0xcc8b527c, "usb_control_msg" },
	{ 0x8414fc46, "usb_serial_generic_open" },
	{ 0x4d79ef23, "usb_autopm_get_interface" },
	{ 0x368cbcd, "usb_autopm_put_interface" },
	{ 0x23ab74c9, "_dev_err" },
	{ 0xd9da47cd, "__dynamic_dev_dbg" },
	{ 0x6a6e05bf, "kstrtou8" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x3b6c41ea, "kstrtouint" },
	{ 0xdfa44c70, "usb_control_msg_recv" },
	{ 0x37bb7a18, "gpiochip_get_data" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x7c173634, "__bitmap_complement" },
	{ 0x8810754a, "_find_first_bit" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x37a0cba, "kfree" },
	{ 0xdbfa5b55, "device_remove_file" },
	{ 0xa077d6fe, "gpiochip_remove" },
	{ 0x5f540977, "kmalloc_caches" },
	{ 0xfa55b3ee, "kmem_cache_alloc_trace" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x87148c85, "_dev_info" },
	{ 0x4c25be7f, "device_create_file" },
	{ 0xd71c15e7, "gpiochip_add_data_with_key" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x98c62dc4, "_dev_warn" },
	{ 0xb9391fc2, "usb_serial_register_drivers" },
	{ 0x336ea657, "usb_serial_deregister_drivers" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "usbserial");

MODULE_ALIAS("usb:v0403p6001d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "321339B88E38D8C90F7CF9F");
