/*
 * dbm.cpp
 *
 *  Created on: Jan 17, 2020
 *      Author: Gavin Chang
 */

#include "dbm.h"
#include "axclib.h"

//#define _SQlite3_Log_

//extern CAxcLog g_log;
//CAxcLog g_log;


char* GetStringFromTime(time_t* ptime) {
	static char s_time[64];
	struct tm* ptm = localtime(ptime);
	strftime(s_time, 64, "%Y-%m-%d %H:%M:%S", ptm);
	return s_time;
}

sqlite3* openDatabase(char* szFilePath) {

	sqlite3 *db;
	int rc = sqlite3_open(szFilePath, &db);

	if (rc) {
		//AxcLogD(g_log, "[SQlite3] Open database '%s' failed!", szFilePath);
		return NULL;
	} else {
#ifdef _SQlite3_Log_
		AxcLogD(g_log, "[SQlite3] Open database");
#endif
		return db;
	}
}

void closeDatabase(sqlite3* db) 
{
	sqlite3_close(db);
#ifdef _SQlite3_Log_
	AxcLogD(g_log, "[SQlite3] Close database");
#endif
}

int queryRecord_callback(void* param, int argc, char** argv, char** colName) 
{
	PSDBITEM pResult = (PSDBITEM)param;
	for( int i=0 ; i<argc ; i++ ) {
#ifdef _SQlite3_Log_
		AxcLogD(g_log, "[SQlite3] %s = %s", colName[i], argv[i]);
#endif
		if (0 == strcmp("title", colName[i])) {
			strcpy(pResult->szTitle, argv[i]);
		} else if (0 == strcmp("value", colName[i])) {
			strcpy(pResult->szValue, argv[i]);
		} else if ( 0 == strcmp("last_updated", colName[i]) ) {
			struct tm tm;
			strptime(argv[i], "%Y-%m-%d %H:%M:%S", &tm);
			pResult->timeLastUpdated = mktime(&tm);
		}
	}
	return 0;
}

bool queryRecord(sqlite3* db, const char* table, const char* title, SDBITEM& result) 
{

	char *szErrMsg = NULL;
	char sql[1024] = {0};
	char *sqlFmt = "SELECT * " \
	               "FROM %s " \
	               "WHERE title = '%s'";
	sprintf(sql, sqlFmt, table, title);

#ifdef _SQlite3_Log_
	AxcLogD(g_log, "[SQlite3] +++queryRecord");
#endif
	int rc = sqlite3_exec(db, sql, queryRecord_callback, &result, &szErrMsg);

	if(rc != SQLITE_OK) {
		sqlite3_free(szErrMsg);
		//AxcLogD(g_log, "[SQlite3] ---queryRecord <Failed>");
		return false;
	}

#ifdef _SQlite3_Log_
	AxcLogD(g_log, "[SQlite3] ---queryRecord <OK>");
#endif
	return true;
}

bool dbmGetValue(char* table, char* title, char* szBuff, int nLen) {
	bool bResult = false;
	char temp[MAX_ITEM_LEN];
	sqlite3* db = openDatabase(DEF_PI640_CONFIG);
	if(db) {
		SDBITEM item = {0};
		if(queryRecord(db, table, title, item)) {
#ifdef _SQlite3_Log_
			AxcLogN(g_log, "[SQlite3] %s=> %s", title, item.szValue);
			AxcLogN(g_log, "[SQlite3] last_updated=> %s", GetStringFromTime(&(item.timeLastUpdated)));
#endif
			if(strlen(item.szValue) < nLen) {
				strcpy(szBuff,item.szValue);
				bResult = true;
			}
		}
		closeDatabase(db);
	}
	return bResult;
}

bool updateRecord(sqlite3* db, const char* table, SDBITEM& data) {

	char *szErrMsg = NULL;
	char sql[1024] = {0};
	char *sqlFmt = "UPDATE %s " \
				   "SET value = '%s', last_updated = '%s' " \
	               "WHERE title = '%s'; " \
				   "SELECT * " \
				   "FROM %s " \
	               "WHERE title = '%s'";
	time_t now;
	time(&now);
	char *szLast_updated = GetStringFromTime(&now);
	sprintf(sql, sqlFmt, table, data.szValue, szLast_updated, data.szTitle, table, data.szTitle);

#ifdef _SQlite3_Log_
	AxcLogD(g_log, "[SQlite3] +++updateRecord");
#endif
	SDBITEM item = {0};
	int rc = sqlite3_exec(db, sql, queryRecord_callback, &item, &szErrMsg);

	if(rc != SQLITE_OK) {
		sqlite3_free(szErrMsg);
		//AxcLogD(g_log, "[SQlite3] ---updateRecord <Failed>");
		return false;
	}

	memcpy( &data, &item, sizeof(item) );

#ifdef _SQlite3_Log_
	AxcLogD(g_log, "[SQlite3] UPDATE");
	AxcLogD(g_log, "[SQlite3]   table: %s", table);
	AxcLogD(g_log, "[SQlite3]   title: %s", data.szTitle);
	AxcLogD(g_log, "[SQlite3]   value: %s", data.szValue);
	AxcLogD(g_log, "[SQlite3]   last_updated: %s", GetStringFromTime(&(data.timeLastUpdated)));
	AxcLogD(g_log, "[SQlite3] ---updateRecord <OK>");
#endif
	return true;
}

bool dbmSetValue(char* table, char* title, char* value) {
	bool bResult = false;
	sqlite3* db = openDatabase(DEF_PI640_CONFIG);
	if(db) {
		SDBITEM item = {0};
		strcpy( item.szTitle, title );
		strcpy( item.szValue, value );
		bResult = updateRecord(db, table, item);
		closeDatabase(db);
	}
	return bResult;
}
