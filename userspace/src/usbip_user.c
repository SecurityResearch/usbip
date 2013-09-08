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
#include "usbip_db.h"

static const char usbip_user_usage_string[] =
	"usbip user <args>\n"
	"    -a, --add=<userid>        add a user\n"
	"    -r, --remove=<userid>     remove a user\n"
    "    -m, --modify=<userid>     modify a user\n"
    "    -p, --passwd=<password>   password of user\n";

void usbip_user_usage(void)
{
	printf("usage: %s", usbip_user_usage_string);
}


static int add_user(char *userid, char *passwd)
{
    if(userid == NULL || passwd == NULL){
        err("userid or password is not provided\n");
        return -1;
    }
    if(usbip_sec_ins_key(userid,passwd))
        return -1;
    else
        return 0;
}

static int remove_user(char *userid)
{
	int rc;
    if(userid == NULL){
        err("userid is not provided\n");
        return -1;
    }

    if(usbip_sec_rem_key(userid))
        return -1;
    else
        return 0;

    return rc;
}

static int modify_user(char *userid, char *passwd)
{
	int rc;
    if(userid == NULL || passwd == NULL){
        err("userid or password is not provided\n");
        return -1;
    }

    if(usbip_sec_set_key(userid,passwd))
        return -1;
    else
        return 0;

    return rc;
}

int usbip_user(int argc, char *argv[])
{
	static const struct option opts[] = {
		{ "add", required_argument, NULL, 'a' },
		{ "remove", required_argument, NULL, 'r' },
		{ "modify", required_argument, NULL, 'm' },
		{ "passwd", required_argument, NULL, 'p' },
		{ NULL,    0,                 NULL,  0  }
	};

	int opt;
	int ret = -1;

    char *user=NULL;
    char *passwd=NULL;
    int action = 0;
	for (;;) {
		opt = getopt_long(argc, argv, "a:r:m:p:", opts, NULL);

		if (opt == -1)
			break;

		switch (opt) {
		case 'a':
			user = optarg;
            action = 1;
            break;
		case 'm':
			user = optarg;
            action = 2;
            break;
		case 'r':
			user = optarg;
            action = 3;
            break;
		case 'p':
			passwd = optarg;
            break;
		default:
			goto err_out;
		}
	}
    if(usbip_db_init()){
        err("Error in opening DB\n");
        return ret;
    }
    switch(action) {
    case 1:
        ret=add_user(user,passwd);
        break;
    case 2:
        ret=modify_user(user,passwd);
        break;
    case 3:
        ret=remove_user(user);
        break;
    default:
        if(usbip_db_close()){
            err("Error in closing DB\n");
        }
        goto err_out;
    }
    if(usbip_db_close()){
        err("Error in closing DB\n");
    }

	return ret;

 err_out:
	usbip_user_usage();
	return ret;
}
