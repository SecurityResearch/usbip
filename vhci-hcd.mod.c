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
	{ 0x38b7dbeb, "kmalloc_caches" },
	{ 0x241fabaa, "usbip_event_happened" },
	{ 0xd02753dc, "usbip_header_correct_endian" },
	{ 0xd1750bc3, "kernel_sendmsg" },
	{ 0xb1a2cd4b, "usb_add_hcd" },
	{ 0x76ebea8, "pv_lock_ops" },
	{ 0xccd837e0, "usb_remove_hcd" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x4a34b57c, "usb_create_hcd" },
	{ 0xd09ea50a, "sock_release" },
	{ 0xcef84c27, "usb_hcd_poll_rh_status" },
	{ 0x9bfa0785, "usbip_alloc_iso_desc_pdu" },
	{ 0x399b737b, "usb_hcd_giveback_urb" },
	{ 0x91715312, "sprintf" },
	{ 0xa2d4caf4, "sysfs_remove_group" },
	{ 0xefb12c35, "kthread_create_on_node" },
	{ 0x7d11c268, "jiffies" },
	{ 0x24b510b2, "usb_put_hcd" },
	{ 0xc50b86ca, "usbip_event_add" },
	{ 0x68dfc59f, "__init_waitqueue_head" },
	{ 0xe1ea0586, "usbip_dump_header" },
	{ 0xcc820b5a, "usb_hcd_link_urb_to_ep" },
	{ 0x6198cad9, "dev_err" },
	{ 0xf97456ea, "_raw_spin_unlock_irqrestore" },
	{ 0xf8bff81e, "current_task" },
	{ 0x50eedeb8, "printk" },
	{ 0x20c55ae0, "sscanf" },
	{ 0x21eb73c6, "kthread_stop" },
	{ 0xa1913a7e, "sysfs_create_group" },
	{ 0xb4390f9a, "mcount" },
	{ 0x2bc9c772, "kernel_sock_shutdown" },
	{ 0x9f51aba9, "usbip_stop_eh" },
	{ 0x85f919ed, "dev_attr_usbip_debug" },
	{ 0xb0fc0f27, "platform_device_unregister" },
	{ 0x501a10ec, "platform_driver_register" },
	{ 0x26fb276d, "usbip_recv_iso" },
	{ 0x93bede50, "sockfd_to_socket" },
	{ 0x89e0420, "usbip_pad_iso" },
	{ 0x5f6f6ff5, "usb_hcd_check_unlink_urb" },
	{ 0x85ff6470, "_dev_info" },
	{ 0x8ff4079b, "pv_irq_ops" },
	{ 0x28b5879f, "usbip_dump_urb" },
	{ 0x2a9d4b4d, "platform_device_register" },
	{ 0xc8dd29cd, "usb_get_dev" },
	{ 0xcfb9851b, "usbip_recv" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0xfeced5fa, "usb_put_dev" },
	{ 0x4292364c, "schedule" },
	{ 0xf1faac3a, "_raw_spin_lock_irq" },
	{ 0x2acbc8cd, "wake_up_process" },
	{ 0x7ffde7b, "kmem_cache_alloc_trace" },
	{ 0x67f7403e, "_raw_spin_lock" },
	{ 0x21fb443e, "_raw_spin_lock_irqsave" },
	{ 0xe45f60d8, "__wake_up" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0x19a304ba, "usb_disabled" },
	{ 0x37a0cba, "kfree" },
	{ 0x622fa02a, "prepare_to_wait" },
	{ 0xe7ecc2c0, "usbip_recv_xbuff" },
	{ 0x86930f0f, "__put_task_struct" },
	{ 0x78b72f44, "usbip_debug_flag" },
	{ 0x75bb675a, "finish_wait" },
	{ 0xb151e5de, "platform_driver_unregister" },
	{ 0x49cf91cf, "usb_hcd_unlink_urb_from_ep" },
	{ 0x7cc2b57e, "usb_hcd_resume_root_hub" },
	{ 0xcce551ce, "usbip_start_eh" },
	{ 0x51461818, "dev_get_drvdata" },
	{ 0xdf2224db, "usbip_pack_pdu" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=usbip-core";


MODULE_INFO(srcversion, "745D2849F895ABCBAC44A6A");
