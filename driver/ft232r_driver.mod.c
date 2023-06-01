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
	{ 0x3edc6527, "usb_put_dev" },
	{ 0x37a0cba, "kfree" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xdfb60745, "usb_free_coherent" },
	{ 0xd9da47cd, "__dynamic_dev_dbg" },
	{ 0x92997ed8, "_printk" },
	{ 0x3c913dfb, "usb_alloc_urb" },
	{ 0x185cebe7, "usb_alloc_coherent" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x2de8eb5f, "usb_submit_urb" },
	{ 0xceadc1c3, "usb_free_urb" },
	{ 0x194d777e, "usb_bulk_msg" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x9cdfddac, "usb_find_interface" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0xa82d41ff, "usb_deregister_dev" },
	{ 0x87148c85, "_dev_info" },
	{ 0x5f540977, "kmalloc_caches" },
	{ 0xfa55b3ee, "kmem_cache_alloc_trace" },
	{ 0xcd1af976, "usb_get_dev" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xe638dc6a, "usb_register_dev" },
	{ 0xaca7784b, "usb_register_driver" },
	{ 0x6d7d5d36, "usb_deregister" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v0403p6001d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "9FD5315E64F30AB24256982");
