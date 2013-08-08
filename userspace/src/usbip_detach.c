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

#include <limits.h>

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <unistd.h>

#include "vhci_driver.h"
#include "usbip_common.h"
#include "usbip_network.h"
#include "usbip.h"

static const char usbip_detach_usage_string[] =
	"usbip detach <args>\n"
	"    -p, --port=<port>    " USBIP_VHCI_DRV_NAME
	" port the device is on\n"
    "    -u, --userid=<userid>   The User ID of the client\n"
    "    -w, --passwd=<password> Password of user to authenticate\n";

void usbip_detach_usage(void)
{
	printf("usage: %s", usbip_detach_usage_string);
}

int get_imported_dev_addr(int rhport,char *host,char *busid)
{
  FILE *file;
  char path[PATH_MAX+1];
  char port[20];
  int ret=0;
  snprintf(path, PATH_MAX, "%s/port%d",VHCI_STATE_PATH, rhport);
  
  file = fopen(path, "r");
  if (!file) {
    err("fopen");
    return -1;
  }
  
  ret = fscanf(file, "%s %s %s\n", host, port, busid);
  
  fclose(file);
  
  return ret;
  
}

static int release_busid(char *host,char *busid,char *userid, char *passwd)
{
    int rc = 0;
    struct op_import_request request;
    struct op_import_reply   reply;
    unsigned char crypt_reply[sizeof(reply)+AES_BLOCK_SIZE], *decrypt_reply;
    uint16_t code = OP_REP_RELEASE;
    int sockfd;
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

    /* the key_data is read from the argument list */
    key_data = (unsigned char *)passwd;
    key_data_len = strlen(passwd);
    /* gen key and iv. init the cipher ctx object */
    if (aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) {
        printf("Couldn't initialize AES cipher\n");
        return -1;
    }

    info("Releasing port %s at host %s\n", busid, host);
    sockfd = usbip_net_tcp_connect(host, USBIP_PORT_STRING);
    if (sockfd < 0) {
        err("tcp connect");
        return -1;
    }

    memset(&request, 0, sizeof(request));
    memset(&reply, 0, sizeof(reply));

    /* send a request */
    rc = usbip_net_send_op_common(sockfd, OP_REQ_RELEASE, 0);
    if (rc < 0) {
        err("send op_common");
        return -1;
    }

    input_len = strlen(busid)+1;
    cipher_busid = aes_encrypt(&en, (unsigned char *)busid, &input_len);

	memcpy(request.busid, cipher_busid, input_len+AES_BLOCK_SIZE);
    free(cipher_busid);
    strncpy(request.userid, userid, SYSFS_BUS_ID_SIZE-1);
  

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
    //rc = usbip_net_recv(sockfd, (void *) &reply, sizeof(reply));
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

    info("Releasing port %s at host %s\n", busid, host);
  
    return rc;
}

static int detach_port(char *port,char *userid, char *passwd)
{
	int ret;
	uint8_t portnum;
	char busid[SYSFS_BUS_ID_SIZE];
	char host[20];
	for (unsigned int i=0; i < strlen(port); i++)
		if (!isdigit(port[i])) {
			err("invalid port %s", port);
			return -1;
		}

	/* check max port */

	portnum = atoi(port);

	ret = usbip_vhci_driver_open();
	if (ret < 0) {
		err("open vhci_driver");
		return -1;
	}

	//ret = usbip_vhci_detach_device(portnum);
	ret=get_imported_dev_addr(portnum,host,busid);
	if (ret < 0){
		ret = -1;
		goto done;
	}
	ret=release_busid(host,busid,userid,passwd);
	if (ret < 0){
		ret = -1;
		goto done;
	}

	ret = usbip_vhci_detach_device(portnum);
	if (ret < 0)
		ret= -1;
 done:	
	usbip_vhci_driver_close();
	return ret;
}

int usbip_detach(int argc, char *argv[])
{
	static const struct option opts[] = {
		{ "port", required_argument, NULL, 'p' },
		{ "userid", required_argument, NULL, 'u' },
		{ "passwd", required_argument, NULL, 'w' },
		{ NULL, 0, NULL, 0 }
	};
    char *userid= NULL;
    char *passwd=NULL;
    char *port=NULL;
	int opt;
	int ret = -1;

	for (;;) {
		opt = getopt_long(argc, argv, "p:u:w:", opts, NULL);

		if (opt == -1)
			break;

		switch (opt) {
		case 'p':
            port = optarg;
			break;
		case 'u':
			userid = optarg;
			break;
		case 'w':
			passwd = optarg;
			break;
		default:
			goto err_out;
		}
	}

	if (!port || !userid || !passwd)
		goto err_out;
    ret = detach_port(port,userid,passwd);
    goto out;
err_out:
	usbip_detach_usage();
out:
	return ret;
}
