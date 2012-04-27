#include "bedFile.h"

void SqlFile::Open(void) {
    std::string sql_exec;
    sqlite3_open_v2(bedFile.c_str(), &_db, SQLITE_OPEN_READONLY, NULL);
    sql_exec = std::string("select name,length from 'chrNames'"); 
    int rc = sqlite3_prepare_v2(_db, sql_exec.c_str(), -1, &_stmt, 0);
    _chrom_table.clear();
    while (sqlite3_step(_stmt) == SQLITE_ROW) {
        _chrom_table[std::string((char*)sqlite3_column_text(_stmt, 0))] = (long)sqlite3_column_int(_stmt, 1);
    }
    I_chrom = _chrom_table.begin();
    prepareNextChrom(I_chrom->first);
}

void SqlFile::Close(void) {
    _chrom_table.clear();
    sqlite3_finalize(_stmt);
    sqlite3_close(_db);
}

inline void SqlFile::prepareNextChrom( const std::string &chrom ) {
    _prev_chrom = chrom;
    int rc = sqlite3_finalize(_stmt);
    std::string sql_exec = std::string("select * from '")+_prev_chrom+std::string("' order by start, end, name"); 
    rc = sqlite3_prepare_v2(_db, sql_exec.c_str(), -1, &_stmt, 0);
}

bool SqlFile::GetNextBed(BED &bed, bool forceSorted) {
    int rc = sqlite3_step(_stmt);
    int ncol;
    std::string sql_exec;
    _status = BED_INVALID;
    switch( rc ) {
    case SQLITE_DONE:
        I_chrom++;
        if (I_chrom == _chrom_table.end()) {
            _status = BED_BLANK;
            return false;
        }
        prepareNextChrom(I_chrom->first);
        return GetNextBed(bed, forceSorted);
        break;
    case SQLITE_ROW:
        bed.chrom = _prev_chrom;
        bed.start = (CHRPOS)sqlite3_column_int(_stmt, 0);
        bed.end = (CHRPOS)sqlite3_column_int(_stmt, 1);
        bed.fields.clear();
        ncol = 2;
        while (1) {
            const char *_name = sqlite3_column_name(_stmt, ncol);
            if (_name == NULL) break;
            if (std::string("name").compare(_name) == 0) 
                bed.name = std::string((char*)sqlite3_column_text(_stmt, ncol));
            else if (std::string("score").compare(_name) == 0) 
                bed.score = std::string((char*)sqlite3_column_text(_stmt, ncol));
            else if (std::string("strand").compare(_name) == 0) 
                bed.strand = std::string(sqlite3_column_int(_stmt, ncol)>0 ? "+" : "-");
            else 
                bed.fields.push_back(std::string((char*)sqlite3_column_text(_stmt, ncol)));
            ncol++;
        }
        _prev_start = bed.start;
        _status = BED_VALID;
        return true;
    default:
        std::cerr << "Error: " << rc << ": " << sqlite3_errmsg(_db) << "\n";
        break;
    }
    return false;
}


