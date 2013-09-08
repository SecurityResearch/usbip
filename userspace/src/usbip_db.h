#include <stdio.h>
#include "sqlite3.h"


sqlite3 *usbip_db;

int usbip_db_init();
int usbip_db_close();

unsigned char * usbip_sec_get_key(char *);
int usbip_sec_set_key(char *,char *);
int usbip_sec_ins_key(char *,char *);
int usbip_sec_rem_key(char *);
