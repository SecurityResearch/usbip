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

#include <sysfs/libsysfs.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "usbip_common.h"
#include "utils.h"
#include "usbip.h"

static const char usbip_disable_usage_string[] =
	"usbip disable <args>\n"
	"    -b, --busid=<busid>    Disable"
	"the <busid>\n";

void usbip_disable_usage(void)
{
	printf("usage: %s", usbip_disable_usage_string);
}


static int disable_busid(char *busid)
{
	int rc;

	//rc = modify_match_busid(busid, 1);
	rc = modify_hub_port(busid, USB_PORT_DISABLE);
	if (rc < 0) {
		err("unable to disable busid %s", busid);
		return -1;
	}
    
    printf("Disabled busid %s: complete\n", busid);

    return 0;
}

int usbip_disable(int argc, char *argv[])
{
	static const struct option opts[] = {
		{ "busid", required_argument, NULL, 'b' },
		{ NULL,    0,                 NULL,  0  }
	};

	int opt;
	int ret = -1;

	for (;;) {
		opt = getopt_long(argc, argv, "b:", opts, NULL);

		if (opt == -1)
			break;

		switch (opt) {
		case 'b':
			ret = disable_busid(optarg);
			goto out;
		default:
			goto err_out;
		}
	}

err_out:
	usbip_disable_usage();
out:
	return ret;
}
