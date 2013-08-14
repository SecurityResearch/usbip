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
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

#include "vhci_driver.h"
#include "usbip_common.h"
#include "usbip_network.h"
#include "usbip.h"

static const char usbip_attach_usage_string[] =
	"usbip attach <args>\n"
	"    -h, --host=<host>       The machine with exported USB devices\n"
	"    -b, --busid=<busid>     Busid of the device on <host>\n"
    "    -u, --userid=<userid>   The User ID of the client\n"
    "    -p, --passwd=<password> Password of user to authenticate\n";

void usbip_attach_usage(void)
{
	printf("usage: %s", usbip_attach_usage_string);
}

#define MAX_BUFF 100
static int record_connection(char *host, char *port, char *busid, int rhport)
{
	int fd;
	char path[PATH_MAX+1];
	char buff[MAX_BUFF+1];
	int ret;
	struct stat file_status;
	ret = stat(VHCI_STATE_PATH,&file_status);
	if(ret<0){
	  ret = mkdir(VHCI_STATE_PATH, 0700);
	}
	if (ret < 0)
		return -1;

	snprintf(path, PATH_MAX, VHCI_STATE_PATH"/port%d", rhport);

	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
	if (fd < 0)
		return -1;

	snprintf(buff, MAX_BUFF, "%s %s %s\n",
			host, port, busid);

	ret = write(fd, buff, strlen(buff));
	if (ret != (ssize_t) strlen(buff)) {
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static int import_device(int sockfd, struct usbip_usb_device *udev, char *passwd)
{
	int rc;
	int port;

	rc = usbip_vhci_driver_open();
	if (rc < 0) {
		err("open vhci_driver");
		return -1;
	}

	port = usbip_vhci_get_free_port();
	if (port < 0) {
		err("no free port");
		usbip_vhci_driver_close();
		return -1;
	}

    struct timeval tv;

    tv.tv_sec = 3;  /* 3 Secs Timeout */
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
 
	rc = usbip_vhci_attach_device(port, sockfd, udev->busnum,
                                  udev->devnum, udev->speed, passwd);
	if (rc < 0) {
		err("import device");
		usbip_vhci_driver_close();
		return -1;
	}

	usbip_vhci_driver_close();

	return port;
}

static int query_import_device(int sockfd, char *busid, char *id, char *passwd)
{
	int rc;
	struct op_import_request request;
	struct op_import_reply   reply;
    unsigned char crypt_reply[sizeof(reply)+AES_BLOCK_SIZE], *decrypt_reply;
	uint16_t code = OP_REP_IMPORT;
    //char *pwd=passwd;/***/
    /* "opaque" encryption, decryption ctx structures that libcrypto uses to record
       status of enc/dec operations */
    EVP_CIPHER_CTX en, de;
    
    /* 8 bytes to salt the key_data during key generation. This is an example of
       compiled in salt. We just read the bit pattern created by these two 4 byte 
       integers on the stack as 64 bits of contigous salt material - 
       ofcourse this only works if sizeof(int) >= 4 */
    unsigned int salt[] = {12345, 54321};
    unsigned char *key_data;
    unsigned char *cipher_busid;
    int key_data_len, input_len;
    //memset(pwd,0,strlen(passwd));
    memset(&request, 0, sizeof(request));
	memset(&reply, 0, sizeof(reply));
    /* the key_data is read from the argument list */
    key_data = (unsigned char *)passwd;
    key_data_len = strlen(passwd);
    /* gen key and iv. init the cipher ctx object */
    if (aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) {
        printf("Couldn't initialize AES cipher\n");
        return -1;
    }

	/* send a request */
	rc = usbip_net_send_op_common(sockfd, OP_REQ_IMPORT, 0);
	if (rc < 0) {
		err("send op_common");
		return -1;
	}

    input_len = strlen(busid)+1;
    cipher_busid = aes_encrypt(&en, (unsigned char *)busid, &input_len);

	memcpy(request.busid, cipher_busid, input_len+AES_BLOCK_SIZE);
    free(cipher_busid);
	strncpy(request.userid, id, SYSFS_BUS_ID_SIZE-1);

	PACK_OP_IMPORT_REQUEST(0, &request);

	rc = usbip_net_send(sockfd, (void *) &request, sizeof(request));
	if (rc < 0) {
		err("send op_import_request");
		return -1;
	}

	/* recieve a reply */
	rc = usbip_net_recv_op_common(sockfd, &code);
	if (rc < 0) {
		err("recv op_common");
		return -1;
	}

	rc = usbip_net_recv(sockfd, (void *) &crypt_reply, sizeof(reply)+AES_BLOCK_SIZE);
	if (rc < 0) {
		err("recv op_import_reply");
		return -1;
	}
    input_len = sizeof(reply);
    decrypt_reply = aes_decrypt(&de, crypt_reply, &input_len);
    memcpy(&reply,decrypt_reply,sizeof(reply));
    free(decrypt_reply);

	PACK_OP_IMPORT_REPLY(0, &reply);

	/* check the reply */
	if (strncmp(reply.udev.busid, busid, SYSFS_BUS_ID_SIZE)) {
		err("recv different busid %s", reply.udev.busid);
		return -1;
	}

	/* import a device */
	return import_device(sockfd, &reply.udev, passwd);
}

static int attach_device(char *host, char *busid, char *userid, char *passwd)
{
	int sockfd;
	int rc;
	int rhport;

	sockfd = usbip_net_tcp_connect(host, USBIP_PORT_STRING);
	if (sockfd < 0) {
		err("tcp connect");
		return -1;
	}

	rhport = query_import_device(sockfd, busid,userid,passwd);
	if (rhport < 0) {
		err("query");
		return -1;
	}

	close(sockfd);

	rc = record_connection(host, USBIP_PORT_STRING, busid, rhport);
	if (rc < 0) {
		err("record connection");
		return -1;
	}

	return 0;
}

int usbip_attach(int argc, char *argv[])
{
	static const struct option opts[] = {
		{ "host", required_argument, NULL, 'h' },
		{ "busid", required_argument, NULL, 'b' },
		{ "userid", required_argument, NULL, 'u' },
		{ "passwd", required_argument, NULL, 'p' },
		{ NULL, 0, NULL, 0 }
	};
	char *host = NULL;
	char *busid = NULL;
    char *userid= NULL;
    char *passwd=NULL;
	int opt;
	int ret = -1;

	for (;;) {
		opt = getopt_long(argc, argv, "h:b:u:p:", opts, NULL);

		if (opt == -1)
			break;

		switch (opt) {
		case 'h':
			host = optarg;
			break;
		case 'b':
			busid = optarg;
			break;
		case 'u':
			userid = optarg;
			break;
		case 'p':
			passwd = optarg;
			break;
		default:
			goto err_out;
		}
	}

	if (!host || !busid || !userid || !passwd)
		goto err_out;

	ret = attach_device(host, busid,userid, passwd);
	goto out;

err_out:
	usbip_attach_usage();
out:
	return ret;
}
