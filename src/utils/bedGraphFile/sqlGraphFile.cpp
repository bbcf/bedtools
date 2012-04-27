#include "bedGraphFile.h"

void SqlGraphFile::Open(void) {
    std::string sql_exec;
    sqlite3_open_v2(bedGraphFile.c_str(), &_db, SQLITE_OPEN_READONLY, NULL);
    sql_exec = std::string("select name,length from 'chrNames'"); 
    int rc = sqlite3_prepare_v2(_db, sql_exec.c_str(), -1, &_stmt, 0);
    _chrom_table.clear();
    while (sqlite3_step(_stmt) == SQLITE_ROW) {
        _chrom_table[std::string((char*)sqlite3_column_text(_stmt, 0))] = (long)sqlite3_column_int(_stmt, 1);
    }
    I_chrom = _chrom_table.begin();
    prepareNextChrom(I_chrom->first);
}

void SqlGraphFile::Close(void) {
    _chrom_table.clear();
    sqlite3_finalize(_stmt);
    sqlite3_close(_db);
}





