#include<stdlib.h>
# include "usbip_db.h"

int usbip_db_init()
{
    int ret;
    char *zErrMsg = NULL;
  
    ret = sqlite3_open(USBIP_DB,&usbip_db);
    if( ret ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(usbip_db));
        sqlite3_close(usbip_db);
        return(1);
    }

    char *create_tbl="CREATE TABLE IF NOT EXISTS USBIP_USER_TABLE(Id INTEGER PRIMARY KEY DESC,Key INTEGER);";
    ret = sqlite3_exec(usbip_db, create_tbl, NULL, 0, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    return 0;
}

int usbip_db_close()
{
    int ret;
  
    if(usbip_db == NULL){
        return 0;
    }
    ret = sqlite3_close(usbip_db);
    if( ret ){
        fprintf(stderr, "Can't close database: %s\n", sqlite3_errmsg(usbip_db));
        return(1);
    }
    usbip_db = NULL;
    return 0;
}

int usbip_sec_set_key(int id, int key)
{
    int ret,rc;
    char *zErrMsg = NULL;
    char update_tbl[100];
  

    if(usbip_db == NULL){
        fprintf(stderr, "SQL error: Database not up\n");
        return -1;
    }

    ret = usbip_sec_get_key(id);
    if(ret>=0){
        sprintf(update_tbl,"UPDATE TABLE USBIP_USER_TABLE SET Key=%d WHERE Id=%d;",key,id);
    }else{
        sprintf(update_tbl,"INSERT INTO USBIP_USER_TABLE(Id,Key) VALUES(%d,%d);",key,id);
    }
    rc = sqlite3_exec(usbip_db, update_tbl, NULL, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    return 0;
}

int usbip_sec_get_key(int id)
{

    int ret;
    char *zErrMsg = NULL;
    char **result;
    char find_user[100];
    int nRows, nCols;
    char *temp="SELECT Key FROM USBIP_USER_TABLE WHERE Id=%d;";
    sprintf(find_user,temp,id);
    ret = sqlite3_get_table(usbip_db,find_user , &result, &nRows, &nCols, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_free_table(result);
        return -1;
    }

    if(nRows > 0){
        return atoi(result[1]);
    }

    return -1;
}
