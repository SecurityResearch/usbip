#include<stdlib.h>
#include<string.h>
# include "usbip_db.h"

int usbip_db_init()
{
    int ret;
    char *zErrMsg = NULL;
    char *db_env = "USBIP_DB";
    char *db;
    setenv(db_env,"/var/lib/usbip/usbip.db",0);
    db = getenv(db_env);
    ret = sqlite3_open(db,&usbip_db);
    if( ret ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(usbip_db));
        sqlite3_close(usbip_db);
        return(1);
    }

    char *create_tbl="CREATE TABLE IF NOT EXISTS USBIP_USER_TABLE(Id TEXT PRIMARY KEY DESC,Key TEXT);";
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

int usbip_sec_ins_key(char *id, char *key)
{
    int rc;
    char *zErrMsg = NULL;
    char update_tbl[100];
  

    if(usbip_db == NULL){
        fprintf(stderr, "SQL error: Database not up\n");
        return -1;
    }

    sprintf(update_tbl,"INSERT INTO USBIP_USER_TABLE(Id,Key) VALUES('%s','%s');",id,key);
    rc = sqlite3_exec(usbip_db, update_tbl, NULL, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    return 0;
}

int usbip_sec_set_key(char *id, char *key)
{
    int rc;
    char *zErrMsg = NULL;
    char update_tbl[100];
  

    if(usbip_db == NULL){
        fprintf(stderr, "SQL error: Database not up\n");
        return -1;
    }

    sprintf(update_tbl,"UPDATE USBIP_USER_TABLE SET Key='%s' WHERE Id='%s';",key,id);
    rc = sqlite3_exec(usbip_db, update_tbl, NULL, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    return 0;
}

unsigned char * usbip_sec_get_key(char *id)
{

    int ret;
    char *zErrMsg = NULL;
    char **result;
    char find_key[100];
    int nRows, nCols;
    unsigned char *key=NULL;
    char *temp="SELECT Key FROM USBIP_USER_TABLE WHERE Id='%s';";
    sprintf(find_key,temp,id);
    ret = sqlite3_get_table(usbip_db,find_key , &result, &nRows, &nCols, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_free_table(result);
        return NULL;
    }

    if(nRows > 0){
        key = (unsigned char *)strdup(result[1]);
        return key;
    }

    return NULL;
}

int usbip_sec_rem_key(char *id)
{

    int ret;
    char *zErrMsg = NULL;
    char **result;
    char find_key[100];
    int nRows, nCols;
    char *temp="DELETE FROM USBIP_USER_TABLE WHERE Id=%s;";
    sprintf(find_key,temp,id);
    ret = sqlite3_get_table(usbip_db,find_key , &result, &nRows, &nCols, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_free_table(result);
        return -1;
    }

    return 0;
}
