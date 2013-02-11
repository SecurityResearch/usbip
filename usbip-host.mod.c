#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

MODULE_INFO(staging, "Y");

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8a2341e1, "module_layout" },
	{ 0x6cdc8b6, "kmem_cache_destroy" },
	{ 0x9a33f976, "device_remove_file" },
	{ 0x38b7dbeb, "kmalloc_caches" },
	{ 0x241fabaa, "usbip_event_happened" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xd02753dc, "usbip_header_correct_endian" },
	{ 0xd1750bc3, "kernel_sendmsg" },
	{ 0x76ebea8, "pv_lock_ops" },
	{ 0xd6d0acd6, "dev_set_drvdata" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xd09ea50a, "sock_release" },
	{ 0x9bfa0785, "usbip_alloc_iso_desc_pdu" },
	{ 0xcb32c9e4, "usb_kill_urb" },
	{ 0xe2fae716, "kmemdup" },
	{ 0xaa5eaedd, "mutex_unlock" },
	{ 0x91715312, "sprintf" },
	{ 0xd0496eb3, "usb_unlink_urb" },
	{ 0xefb12c35, "kthread_create_on_node" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xc50b86ca, "usbip_event_add" },
	{ 0x68dfc59f, "__init_waitqueue_head" },
	{ 0xe1ea0586, "usbip_dump_header" },
	{ 0x7b5dab9d, "usb_lock_device_for_reset" },
	{ 0x6198cad9, "dev_err" },
	{ 0xf97456ea, "_raw_spin_unlock_irqrestore" },
	{ 0xf8bff81e, "current_task" },
	{ 0xe98b2038, "usb_deregister" },
	{ 0x50eedeb8, "printk" },
	{ 0x20c55ae0, "sscanf" },
	{ 0x21eb73c6, "kthread_stop" },
	{ 0xfbe19c9, "usb_set_interface" },
	{ 0xb6ed1e53, "strncpy" },
	{ 0xb4390f9a, "mcount" },
	{ 0x6c2e3320, "strncmp" },
	{ 0x6e7d3d90, "kmem_cache_free" },
	{ 0x2bc9c772, "kernel_sock_shutdown" },
	{ 0x9f51aba9, "usbip_stop_eh" },
	{ 0x85f919ed, "dev_attr_usbip_debug" },
	{ 0x26fb276d, "usbip_recv_iso" },
	{ 0x93bede50, "sockfd_to_socket" },
	{ 0xeae58e84, "device_create_file" },
	{ 0x85ff6470, "_dev_info" },
	{ 0x50701c19, "usb_submit_urb" },
	{ 0x383fc083, "kmem_cache_alloc" },
	{ 0x28b5879f, "usbip_dump_urb" },
	{ 0xc8dd29cd, "usb_get_dev" },
	{ 0x738803e6, "strnlen" },
	{ 0x8e4295f2, "driver_create_file" },
	{ 0xcfb9851b, "usbip_recv" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xbdcbda41, "usb_reset_device" },
	{ 0xfeced5fa, "usb_put_dev" },
	{ 0x4292364c, "schedule" },
	{ 0x61c44465, "usb_clear_halt" },
	{ 0x2acbc8cd, "wake_up_process" },
	{ 0x7ffde7b, "kmem_cache_alloc_trace" },
	{ 0x67f7403e, "_raw_spin_lock" },
	{ 0xccd0a7e7, "usb_get_intf" },
	{ 0x21fb443e, "_raw_spin_lock_irqsave" },
	{ 0xade55b77, "kmem_cache_create" },
	{ 0xe45f60d8, "__wake_up" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0x37a0cba, "kfree" },
	{ 0x622fa02a, "prepare_to_wait" },
	{ 0xe7ecc2c0, "usbip_recv_xbuff" },
	{ 0x86930f0f, "__put_task_struct" },
	{ 0xbfe2d402, "usb_register_driver" },
	{ 0x78b72f44, "usbip_debug_flag" },
	{ 0x75bb675a, "finish_wait" },
	{ 0x3c9e99e9, "driver_remove_file" },
	{ 0xb81960ca, "snprintf" },
	{ 0xcce551ce, "usbip_start_eh" },
	{ 0x51461818, "dev_get_drvdata" },
	{ 0x78cd9578, "usb_free_urb" },
	{ 0xdf2224db, "usbip_pack_pdu" },
	{ 0x52770bcb, "usb_alloc_urb" },
	{ 0x9ee25614, "usb_put_intf" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=usbip-core";


MODULE_INFO(srcversion, "E5A7189F8AEC57A04749D22");
