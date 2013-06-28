/*
 * Copyright (C) 2011 matt mooney <mfm@muteddisk.com>
 *               2005-2007 Takahiro Hirofuchi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <sysfs/libsysfs.h>

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

#include <glib.h>

#include <netdb.h>


#include "vhci_driver.h"
#include "usbip_common.h"
#include "usbip_network.h"
#include "usbip.h"

static const char usbip_port_usage_string[] =
	"usbip port";

void usbip_port_usage(void)
{
	printf("usage: %s", usbip_port_usage_string);
}

static int read_record(int rhport, char *host, char *port, char *busid)
{
	FILE *file;
	char path[PATH_MAX+1];

	snprintf(path, PATH_MAX, "%s/port%d",VHCI_STATE_PATH, rhport);

	file = fopen(path, "r");
	if (!file) {
		err("fopen");
		return -1;
	}

	int ret = fscanf(file, "%s %s %s\n", host, port, busid);

	fclose(file);

	return ret;
}

static struct sysfs_device *open_usb_interface(struct usbip_usb_device *udev, int i)
{
	struct sysfs_device *suinf;
	char busid[SYSFS_BUS_ID_SIZE];

	snprintf(busid, SYSFS_BUS_ID_SIZE, "%s:%d.%d",
			udev->busid, udev->bConfigurationValue, i);

	suinf = sysfs_open_device("usb", busid);
	if (!suinf)
		err("sysfs_open_device %s", busid);

	return suinf;
}

/* /sys/devices/platform/vhci_hcd/usb6/6-1/6-1:1.1  -> 1 */
static int get_interface_number(char *path)
{
	char *c;

	c = strstr(path, vhci_driver->hc_device->bus_id);
	if (!c)
		return -1;	/* hc exist? */
	c++;
	/* -> usb6/6-1/6-1:1.1 */

	c = strchr(c, '/');
	if (!c)
		return -1;	/* hc exist? */
	c++;
	/* -> 6-1/6-1:1.1 */

	c = strchr(c, '/');
	if (!c)
		return -1;	/* no interface path */
	c++;
	/* -> 6-1:1.1 */

	c = strchr(c, ':');
	if (!c)
		return -1;	/* no configuration? */
	c++;
	/* -> 1.1 */

	c = strchr(c, '.');
	if (!c)
		return -1;	/* no interface? */
	c++;
	/* -> 1 */


	return atoi(c);
}



void usbip_port_dump(struct usbip_imported_device *idev)
{
	char product_name[100];
	char host[NI_MAXHOST] = "unknown host";
	char serv[NI_MAXSERV] = "unknown port";
	char remote_busid[SYSFS_BUS_ID_SIZE];
	int ret;

	if (idev->status == VDEV_ST_NULL || idev->status == VDEV_ST_NOTASSIGNED) {
		info("Port %02d: <%s>", idev->port, usbip_status_string(idev->status));
		return;
	}

	ret = read_record(idev->port, host, serv, remote_busid);
	if (ret)
		err("red_record");

	info("Port %02d: <%s> at %s", idev->port,
			usbip_status_string(idev->status), usbip_speed_string(idev->udev.speed));

	usbip_names_get_product(product_name, sizeof(product_name),
			idev->udev.idVendor, idev->udev.idProduct);

	info("       %s",  product_name);

	info("%10s -> usbip://%s:%s/%s  (remote devid %08x (bus/dev %03d/%03d))",
			idev->udev.busid, host, serv, remote_busid,
			idev->devid,
			idev->busnum, idev->devnum);

	for (int i=0; i < idev->udev.bNumInterfaces; i++) {
		//show interface information 
		struct sysfs_device *suinf;

		suinf = open_usb_interface(&idev->udev, i);
		if (!suinf)
			continue;

		info("       %6s used by %-17s", suinf->bus_id, suinf->driver_name);
		sysfs_close_device(suinf);

		// show class device information
		struct usbip_class_device *cdev;

		dlist_for_each_data(idev->cdev_list, cdev, struct usbip_class_device) {
			int ifnum = get_interface_number(cdev->dev_path);
			if (ifnum == i) {
				info("           %s", cdev->class_path);
			}
		}
	}
}


void usbip_vhci_show_port_status(void)
{
	struct usbip_imported_device *idev;

	for (int i = 0; i < vhci_driver->nports; i++) {
		idev = &vhci_driver->idev[i];

		usbip_port_dump(idev);
	}

}


static void show_port_status(void)
{
	int ret;
	//struct usbip_imported_device *idev;

	ret = usbip_vhci_driver_open();
	if (ret < 0) {
		err("open vhci_driver");
		return;
	}


	usbip_vhci_show_port_status();

	usbip_vhci_driver_close();
}


int usbip_port(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

	int ret = 0;

	show_port_status();

	return ret;
}

