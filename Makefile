ccflags-$(CONFIG_USBIP_DEBUG) := -DDEBUG

obj-$(CONFIG_USBIP_CORE) += usbip-core.o
usbip-core-y := usbip_common.o usbip_event.o

obj-$(CONFIG_USBIP_VHCI_HCD11) += vhci-hcd11.o
vhci-hcd11-y := vhci_sysfs.o vhci_tx.o vhci_rx.o vhci_hcd11.o

obj-$(CONFIG_USBIP_VHCI_HCD2) += vhci-hcd2.o
vhci-hcd2-y := vhci_sysfs2.o vhci_tx2.o vhci_rx2.o vhci_hcd2.o

obj-$(CONFIG_USBIP_HOST) += usbip-host.o
usbip-host-y := stub_dev.o stub_main.o stub_rx.o stub_tx.o
