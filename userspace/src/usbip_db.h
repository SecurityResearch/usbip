#include <stdio.h>
#include "sqlite3.h"

#define USBIP_DB "usbip.db"

sqlite3 *usbip_db;

int usbip_db_init();
int usbip_db_close();

int usbip_sec_get_key(int id);
int usbip_sec_set_key(int id,int key);
