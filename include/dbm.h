/*
 * dbm.h
 *
 *  Created on: Jan 17, 2020
 *      Author: Gavin Chang
 */

#ifndef DBM_H_
#define DBM_H_

#include <sqlite3.h>
#include <time.h>

#define DEF_PI640_CONFIG "/home/ubilinux/heatFinderSvr/.persistStore/pi640.db"
#define MAX_ITEM_LEN	128

/**
 * The struct for manipulate each setting record with database
 */
typedef struct tagDBItem {
	char szTitle[MAX_ITEM_LEN];
	char szValue[MAX_ITEM_LEN];
	time_t timeLastUpdated;
} SDBITEM, *PSDBITEM;


sqlite3* openDatabase(char* szFilePath);
void closeDatabase(sqlite3* db);
bool queryRecord(sqlite3* db, const char* table, const char* title, SDBITEM& result);
bool updateRecord(sqlite3* db, const char* table, SDBITEM& data);

// Convenience function for get time string from time_t
char* GetStringFromTime(time_t* ptime);

/**
 * Quick get a value belongs to table/title
 *
 * CAUTION: If want to get several values continuously, please use queryRecord function
 */
bool dbmGetValue(char* table, char* title, char* szBuff, int nLen);

/**
 * Quick set a value belongs to table/title
 *
 * CAUTION: If want to set several values continuously, please use updateRecord function
 */
bool dbmSetValue(char* table, char* title, char* value);

#endif /* DBM_H_ */
