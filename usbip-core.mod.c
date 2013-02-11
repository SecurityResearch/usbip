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
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x76ebea8, "pv_lock_ops" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x91715312, "sprintf" },
	{ 0xefb12c35, "kthread_create_on_node" },
	{ 0x68dfc59f, "__init_waitqueue_head" },
	{ 0x6198cad9, "dev_err" },
	{ 0xf8bff81e, "current_task" },
	{ 0x50eedeb8, "printk" },
	{ 0x20c55ae0, "sscanf" },
	{ 0x21eb73c6, "kthread_stop" },
	{ 0xb4390f9a, "mcount" },
	{ 0x4059792f, "print_hex_dump" },
	{ 0x4292364c, "schedule" },
	{ 0x2acbc8cd, "wake_up_process" },
	{ 0x67f7403e, "_raw_spin_lock" },
	{ 0x66b4469, "kernel_recvmsg" },
	{ 0xe45f60d8, "__wake_up" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0x37a0cba, "kfree" },
	{ 0x622fa02a, "prepare_to_wait" },
	{ 0x4c6b53e, "fget" },
	{ 0x75bb675a, "finish_wait" },
	{ 0x8235805b, "memmove" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "596235C2983D9D177A770E6");
