#include "sqlGenome.h"
#include <sqlite3.h> 

void SqlGenome::loadGenomeFileIntoMap() {
    std::string sql_exec;
    sqlite3 *_db;
    sqlite3_stmt *_stmt;
    sqlite3_open_v2(_genomeFile.c_str(), &_db, SQLITE_OPEN_READONLY, NULL);
    sql_exec = std::string("select name,length from 'chrNames'"); 
    int rc = sqlite3_prepare_v2(_db, sql_exec.c_str(), -1, &_stmt, 0);
    while (sqlite3_step(_stmt) == SQLITE_ROW) {
	std::string chrom = std::string((char*)sqlite3_column_text(_stmt, 0));
	int size = sqlite3_column_int(_stmt, 1);
	_chromSizes[chrom] = size;
	_chromList.push_back(chrom);
	_startOffsets.push_back(_genomeLength);
	_genomeLength += size;
    }
    sqlite3_finalize(_stmt);
    sqlite3_close(_db);
}
