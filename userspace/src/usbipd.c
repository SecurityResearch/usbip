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

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef HAVE_LIBWRAP
#include <tcpd.h>
#endif

#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>
#include <signal.h>

#include "usbip_host_driver.h"
#include "usbip_common.h"
#include "usbip_network.h"
#include "utils.h"

#undef  PROGNAME
#define PROGNAME "usbipd"
#define MAXSOCKFD 20

GMainLoop *main_loop;

static const char usbip_version_string[] = PACKAGE_STRING;

static const char usbipd_help_string[] =
	"usage: usbipd [options]			\n"
	"	-D, --daemon				\n"
	"		Run as a daemon process.	\n"
	"						\n"
	"	-d, --debug				\n"
	"		Print debugging information.	\n"
	"						\n"
	"	-h, --help 				\n"
	"		Print this help.		\n"
	"						\n"
	"	-v, --version				\n"
	"		Show version.			\n";

static void usbipd_help(void)
{
	printf("%s\n", usbipd_help_string);
}

static int recv_request_import(int sockfd)
{
	struct op_import_request req;
	struct op_common reply;
	struct usbip_exported_device *edev;
	struct usbip_usb_device pdu_udev;
    /* "opaque" encryption, decryption ctx structures that libcrypto uses to record
       status of enc/dec operations */
    EVP_CIPHER_CTX en, de;

    /* 8 bytes to salt the key_data during key generation. This is an example of
       compiled in salt. We just read the bit pattern created by these two 4 byte 
       integers on the stack as 64 bits of contigous salt material - 
       ofcourse this only works if sizeof(int) >= 4 */
    unsigned int salt[] = {12345, 54321};
    unsigned char *key_data;
    int key_data_len,len;
    char *busid;
    unsigned char *ciphertext;

	int found = 0;
	int error = 0;
	int rc;

    unsigned int busnum,portnum;

	memset(&req, 0, sizeof(req));
	memset(&reply, 0, sizeof(reply));
    /* the key_data is read from the argument list */
    key_data = (unsigned char *)"roshan";
    key_data_len = strlen("roshan");
  

    /* gen key and iv. init the cipher ctx object */
    if (aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) {
        printf("Couldn't initialize AES cipher\n");
        return -1;
    }

	rc = usbip_net_recv(sockfd, &req, sizeof(req));
	if (rc < 0) {
		dbg("usbip_net_recv failed: import request");
		return -1;
	}
	PACK_OP_IMPORT_REQUEST(0, &req);

    len = SYSFS_BUS_ID_SIZE;
    busid = (char *)aes_decrypt(&de, req.busid, &len);

    printf("Received request from user %s for bus %s\n",req.userid,busid);

    sscanf(busid,"%u-%u",&busnum,&portnum);
	dlist_for_each_data(host_driver->edev_list, edev,
                        struct usbip_exported_device) {
		//if (!strncmp(req.busid, edev->udev.busid, SYSFS_BUS_ID_SIZE)) {
        if(edev->udev.busnum == busnum){
			info("found requested device: %s", req.busid);
			found = 1;
			break;
		}
    }
    if(!found){
        info("Hub %d not found for busid %s\n",busnum,busid);
        return -1;
    }
    /*rc = check_busid(req.busid);
      if (rc < 0) {
      dbg("usbip check_busid failed: import request");
      return -1;
      }*/
    found = 1;

	if (found) {
		/* should set TCP_NODELAY for usbip */
		usbip_net_set_nodelay(sockfd);

		/* export device needs a TCP/IP socket descriptor */
		//rc = usbip_host_export_device(edev, sockfd);Changed to include portnumber
		rc = usbip_host_export_device(edev, busid, sockfd, key_data);
		if (rc < 0)
			error = 1;
        
	} else {
		info("requested device not found: %s", busid);
		error = 1;
	}
    struct timeval tv;
    
    tv.tv_sec = 3;  /* 3 Secs Timeout */
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
	rc = usbip_net_send_op_common(sockfd, OP_REP_IMPORT,
                                  (!error ? ST_OK : ST_NA));
	if (rc < 0) {
		dbg("usbip_net_send_op_common failed: %#0x", OP_REP_IMPORT);
		return -1;
	}

	if (error) {
		dbg("import request busid %s: failed", busid);
		return -1;
	}

	memcpy(&pdu_udev, &edev->udev, sizeof(pdu_udev));
	usbip_net_pack_usb_device(1, &pdu_udev);
    strncpy(pdu_udev.busid,busid,SYSFS_BUS_ID_SIZE);
    len = sizeof(pdu_udev);
    ciphertext = aes_encrypt(&en, (unsigned char *)(&pdu_udev), &len);
	rc = usbip_net_send(sockfd, ciphertext, sizeof(pdu_udev)+AES_BLOCK_SIZE);
    free(ciphertext);
	if (rc < 0) {
		dbg("usbip_net_send failed: devinfo");
		return -1;
    }

	dbg("import request busid %s: complete", req.busid);

	return 0;
}

/*
  ROSHAN release port request
 */
static int recv_request_release(int sockfd)
{
	struct op_import_request req;
	struct op_common reply;
	struct usbip_exported_device *edev;
	struct usbip_usb_device pdu_udev;
    /* "opaque" encryption, decryption ctx structures that libcrypto uses to record
       status of enc/dec operations */
    EVP_CIPHER_CTX en, de;

    /* 8 bytes to salt the key_data during key generation. This is an example of
       compiled in salt. We just read the bit pattern created by these two 4 byte 
       integers on the stack as 64 bits of contigous salt material - 
       ofcourse this only works if sizeof(int) >= 4 */
    unsigned int salt[] = {12345, 54321};
    unsigned char *key_data;
    int key_data_len,len;
    char *busid;
    unsigned char *ciphertext;
	int found = 0;
	int error = 0;
	int rc;
    int ret=0;
    unsigned int busnum,portnum;

	memset(&req, 0, sizeof(req));
	memset(&reply, 0, sizeof(reply));

    /* the key_data is read from the argument list */
    key_data = (unsigned char *)"roshan";
    key_data_len = strlen("roshan");
  

    /* gen key and iv. init the cipher ctx object */
    if (aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) {
        printf("Couldn't initialize AES cipher\n");
        return -1;
    }

	rc = usbip_net_recv(sockfd, &req, sizeof(req));
	if (rc < 0) {
		dbg("usbip_net_recv failed: import request");
		return -1;
	}
	PACK_OP_RELEASE_REQUEST(0, &req);
    len = SYSFS_BUS_ID_SIZE;
    busid = (char *)aes_decrypt(&de, req.busid, &len);

    printf("Received request from user %s for bus %s\n",req.userid,busid);

    sscanf(busid,"%u-%u",&busnum,&portnum);
    info("Releasing port %s\n",req.busid);
	dlist_for_each_data(host_driver->edev_list, edev,
			    struct usbip_exported_device) {
		//if (!strncmp(req.busid, edev->udev.busid, SYSFS_BUS_ID_SIZE)) {
        if(edev->udev.busnum == busnum){
			info("found requested device: %s", busid);
			found = 1;
			break;
		}
	}

	if (found) {
		/* should set TCP_NODELAY for usbip */
		usbip_net_set_nodelay(sockfd);

		/* unexport device needs a TCP/IP socket descriptor */
		//rc = usbip_host_export_device(edev, sockfd);Changed to include portnumber
		rc = usbip_host_unexport_device(edev, busid, sockfd, key_data);
		if (rc < 0)
			error = 1;
	} else {
		err("requested device not found: %s", busid);
		error = 1;
	}

	rc = usbip_net_send_op_common(sockfd, OP_REP_RELEASE,
				      (!error ? ST_OK : ST_NA));
	if (rc < 0) {
		err("usbip_net_send_op_common failed: %#0x", OP_REP_IMPORT);
		ret= -1;
        goto out;
	}

	if (error) {
		err("release request busid %s: failed", req.busid);
		ret= -1;
        goto out;
	}

	memcpy(&pdu_udev, &edev->udev, sizeof(pdu_udev));
	usbip_net_pack_usb_device(1, &pdu_udev);
    strncpy(pdu_udev.busid,busid,SYSFS_BUS_ID_SIZE);
    len = sizeof(pdu_udev);
    ciphertext = aes_encrypt(&en, (unsigned char *)(&pdu_udev), &len);
	rc = usbip_net_send(sockfd, ciphertext, sizeof(pdu_udev)+AES_BLOCK_SIZE);
    free(ciphertext);

	if (rc < 0) {
		err("usbip_net_send failed: devinfo");
		ret= -1;
        goto out;
	}

	info("release request busid %s: complete", req.busid);
    ret = 0;
 out:
    if(busid != NULL){
        free(busid);
    }
    return ret;
}

static int send_reply_devlist(int connfd)
{
	struct usbip_exported_device *edev;
	struct usbip_usb_device pdu_udev;
	struct usbip_usb_interface pdu_uinf;
	struct op_devlist_reply reply;
	int i;
	int rc;

	reply.ndev = 0;
	/* number of exported devices */
	dlist_for_each_data(host_driver->edev_list, edev,
			    struct usbip_exported_device) {
		reply.ndev += 1;
	}
	info("exportable devices: %d", reply.ndev);

	rc = usbip_net_send_op_common(connfd, OP_REP_DEVLIST, ST_OK);
	if (rc < 0) {
		dbg("usbip_net_send_op_common failed: %#0x", OP_REP_DEVLIST);
		return -1;
	}
	PACK_OP_DEVLIST_REPLY(1, &reply);

	rc = usbip_net_send(connfd, &reply, sizeof(reply));
	if (rc < 0) {
		dbg("usbip_net_send failed: %#0x", OP_REP_DEVLIST);
		return -1;
	}

	dlist_for_each_data(host_driver->edev_list, edev,
			    struct usbip_exported_device) {
		dump_usb_device(&edev->udev);
		memcpy(&pdu_udev, &edev->udev, sizeof(pdu_udev));
		usbip_net_pack_usb_device(1, &pdu_udev);

		rc = usbip_net_send(connfd, &pdu_udev, sizeof(pdu_udev));
		if (rc < 0) {
			dbg("usbip_net_send failed: pdu_udev");
			return -1;
		}

		for (i = 0; i < edev->udev.bNumInterfaces; i++) {
			dump_usb_interface(&edev->uinf[i]);
			memcpy(&pdu_uinf, &edev->uinf[i], sizeof(pdu_uinf));
			usbip_net_pack_usb_interface(1, &pdu_uinf);

			rc = usbip_net_send(connfd, &pdu_uinf,
					    sizeof(pdu_uinf));
			if (rc < 0) {
				dbg("usbip_net_send failed: pdu_uinf");
				return -1;
			}
		}
	}

	return 0;
}

static int recv_request_devlist(int connfd)
{
	struct op_devlist_request req;
	int rc;

	memset(&req, 0, sizeof(req));

	rc = usbip_net_recv(connfd, &req, sizeof(req));
	if (rc < 0) {
		dbg("usbip_net_recv failed: devlist request");
		return -1;
	}

	rc = send_reply_devlist(connfd);
	if (rc < 0) {
		dbg("send_reply_devlist failed");
		return -1;
	}

	return 0;
}

static int recv_pdu(int connfd)
{
	uint16_t code = OP_UNSPEC;
	int ret;

	ret = usbip_net_recv_op_common(connfd, &code);
	if (ret < 0) {
		info("could not receive opcode: %#0x", code);
		return -1;
	}

	ret = usbip_host_refresh_device_list();
	if (ret < 0) {
		info("could not refresh device list: %d", ret);
		return -1;
	}

	info("received request: %#0x(%d)", code, connfd);
	switch (code) {
	case OP_REQ_DEVLIST:
		ret = recv_request_devlist(connfd);
		break;
	case OP_REQ_IMPORT:
		ret = recv_request_import(connfd);
		break;
	case OP_REQ_RELEASE:
		ret = recv_request_release(connfd);
		break;
	case OP_REQ_DEVINFO:
	case OP_REQ_CRYPKEY:
	default:
		err("received an unknown opcode: %#0x", code);
		ret = -1;
	}

	if (ret == 0)
		info("request %#0x(%d): complete", code, connfd);
	else
		info("request %#0x(%d): failed", code, connfd);

	return ret;
}

#ifdef HAVE_LIBWRAP
static int tcpd_auth(int connfd)
{
	struct request_info request;
	int rc;

	request_init(&request, RQ_DAEMON, PROGNAME, RQ_FILE, connfd, 0);
	fromhost(&request);
	rc = hosts_access(&request);
	if (rc == 0)
		return -1;

	return 0;
}
#endif

static int do_accept(int listenfd)
{
	int connfd;
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char host[NI_MAXHOST], port[NI_MAXSERV];
	int rc;

	memset(&ss, 0, sizeof(ss));

	connfd = accept(listenfd, (struct sockaddr *) &ss, &len);
	if (connfd < 0) {
		err("failed to accept connection");
		return -1;
	}

	rc = getnameinfo((struct sockaddr *) &ss, len, host, sizeof(host),
			 port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);
	if (rc)
		err("getnameinfo: %s", gai_strerror(rc));

#ifdef HAVE_LIBWRAP
	rc = tcpd_auth(connfd);
	if (rc < 0) {
		info("denied access from %s", host);
		close(connfd);
		return -1;
	}
#endif
	info("connection from %s:%s", host, port);

	return connfd;
}

gboolean process_request(GIOChannel *gio, GIOCondition condition,
			 gpointer unused_data)
{
	int listenfd;
	int connfd;

	(void) unused_data;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		err("unknown condition");
		BUG();
	}

	if (condition & G_IO_IN) {
		listenfd = g_io_channel_unix_get_fd(gio);
		connfd = do_accept(listenfd);
		if (connfd < 0)
			return TRUE;

		recv_pdu(connfd);
		close(connfd);
	}

	return TRUE;
}

static void log_addrinfo(struct addrinfo *ai)
{
	char hbuf[NI_MAXHOST];
	char sbuf[NI_MAXSERV];
	int rc;

	rc = getnameinfo(ai->ai_addr, ai->ai_addrlen, hbuf, sizeof(hbuf),
			 sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
	if (rc)
		err("getnameinfo: %s", gai_strerror(rc));

	info("listening on %s:%s", hbuf, sbuf);
}

static int listen_all_addrinfo(struct addrinfo *ai_head, int sockfdlist[])
{
	struct addrinfo *ai;
	int ret, nsockfd = 0;

	for (ai = ai_head; ai && nsockfd < MAXSOCKFD; ai = ai->ai_next) {
		sockfdlist[nsockfd] = socket(ai->ai_family, ai->ai_socktype,
					     ai->ai_protocol);
		if (sockfdlist[nsockfd] < 0)
			continue;

		usbip_net_set_reuseaddr(sockfdlist[nsockfd]);
		usbip_net_set_nodelay(sockfdlist[nsockfd]);

		if (sockfdlist[nsockfd] >= FD_SETSIZE) {
			close(sockfdlist[nsockfd]);
			sockfdlist[nsockfd] = -1;
			continue;
		}

		ret = bind(sockfdlist[nsockfd], ai->ai_addr, ai->ai_addrlen);
		if (ret < 0) {
			close(sockfdlist[nsockfd]);
			sockfdlist[nsockfd] = -1;
			continue;
		}

		ret = listen(sockfdlist[nsockfd], SOMAXCONN);
		if (ret < 0) {
			close(sockfdlist[nsockfd]);
			sockfdlist[nsockfd] = -1;
			continue;
		}

		log_addrinfo(ai);
		nsockfd++;
	}

	if (nsockfd == 0)
		return -1;

	dbg("listening on %d address%s", nsockfd, (nsockfd == 1) ? "" : "es");

	return nsockfd;
}

static struct addrinfo *do_getaddrinfo(char *host, int ai_family)
{
	struct addrinfo hints, *ai_head;
	int rc;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = ai_family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	rc = getaddrinfo(host, USBIP_PORT_STRING, &hints, &ai_head);
	if (rc) {
		err("failed to get a network address %s: %s", USBIP_PORT_STRING,
		    gai_strerror(rc));
		return NULL;
	}

	return ai_head;
}

static void signal_handler(int i)
{
	dbg("received signal: code %d", i);

	if (main_loop)
		g_main_loop_quit(main_loop);
}

static void set_signal(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
}

static int do_standalone_mode(gboolean daemonize)
{
	struct addrinfo *ai_head;
	int sockfdlist[MAXSOCKFD];
	int nsockfd;
	int i;

	if (usbip_names_init(USBIDS_FILE))
		err("failed to open %s", USBIDS_FILE);

	if (usbip_host_driver_open()) {
		err("please load " USBIP_CORE_MOD_NAME ".ko and "
		    USBIP_HOST_DRV_NAME ".ko!");
		return -1;
	}

	if (daemonize) {
		if (daemon(0,0) < 0) {
			err("daemonizing failed: %s", strerror(errno));
			return -1;
		}

		usbip_use_syslog = 1;
	}
	set_signal();

	ai_head = do_getaddrinfo(NULL, PF_UNSPEC);
	if (!ai_head)
		return -1;

	info("starting " PROGNAME " (%s)", usbip_version_string);

	nsockfd = listen_all_addrinfo(ai_head, sockfdlist);
	if (nsockfd <= 0) {
		err("failed to open a listening socket");
		return -1;
	}

	for (i = 0; i < nsockfd; i++) {
		GIOChannel *gio;

		gio = g_io_channel_unix_new(sockfdlist[i]);
		g_io_add_watch(gio, (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
			       process_request, NULL);
	}

	main_loop = g_main_loop_new(FALSE, FALSE);
	g_main_loop_run(main_loop);

	info("shutting down " PROGNAME);

	freeaddrinfo(ai_head);
	usbip_host_driver_close();
	usbip_names_free();

	return 0;
}

int main(int argc, char *argv[])
{
	static const struct option longopts[] = {
		{ "daemon",  no_argument, NULL, 'D' },
		{ "debug",   no_argument, NULL, 'd' },
		{ "help",    no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'v' },
		{ NULL,	     0,           NULL,  0  }
	};

	enum {
		cmd_standalone_mode = 1,
		cmd_help,
		cmd_version
	} cmd;

	gboolean daemonize = FALSE;
	int opt, rc = -1;

	usbip_use_stderr = 1;
	usbip_use_syslog = 0;

	if (geteuid() != 0)
		err("not running as root?");

	cmd = cmd_standalone_mode;
	for (;;) {
		opt = getopt_long(argc, argv, "Ddhv", longopts, NULL);

		if (opt == -1)
			break;

		switch (opt) {
		case 'D':
			daemonize = TRUE;
			break;
		case 'd':
			usbip_use_debug = 1;
			break;
		case 'h':
			cmd = cmd_help;
			break;
		case 'v':
			cmd = cmd_version;
			break;
		case '?':
			usbipd_help();
		default:
			goto err_out;
		}
	}

	switch (cmd) {
	case cmd_standalone_mode:
		rc = do_standalone_mode(daemonize);
		break;
	case cmd_version:
		printf(PROGNAME " (%s)\n", usbip_version_string);
		rc = 0;
		break;
	case cmd_help:
		usbipd_help();
		rc = 0;
		break;
	default:
		usbipd_help();
		goto err_out;
	}

err_out:
	return (rc > -1 ? EXIT_SUCCESS : EXIT_FAILURE);
}
