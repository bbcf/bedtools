#include "sqlFile.h"

void SqlFile::Open(void) {
    // TODO: handle error
    sqlite3_open_v2(bedFile.c_str(), &_db, SQLITE_OPEN_READONLY, NULL);
    /****
         TODO: read chromosome list, switch to next when done with one, etc.
         TODO: variable field set, blablabla
    ****/
    std::string sql_exec("select start, end, name, score from 'chr1' order by start,end,name"); 
    int rc = sqlite3_prepare_v2(_db, sql_exec.c_str(), -1, &_stmt, 0);
    _prev_chrom = std::string("chr1");
}

void SqlFile::Close(void) {
    sqlite3_finalize(_stmt);
    sqlite3_close(_db);
}

bool SqlFile::GetNextBed(BED &bed, bool forceSorted) {
    int rc = sqlite3_step(_stmt);
    _status = BED_INVALID;
    switch( rc ) {
    case SQLITE_DONE:
        /*** TODO: finalize, then start next chrom ***/
        // int rc = sqlite3_reset(_stmt);
        // _prev_chrom = "chr?";
        //return GetNextBed(bed, forceSorted);
        _status = BED_BLANK;
        return false;
    case SQLITE_ROW:
        bed.chrom = std::string("chr1");
        bed.start = (CHRPOS)sqlite3_column_int(_stmt, 0);
        bed.end = (CHRPOS)sqlite3_column_int(_stmt, 1);
        bed.name = std::string((char*)sqlite3_column_text(_stmt, 2));
        bed.score = std::string((char*)sqlite3_column_text(_stmt, 3));
//        bed.strand = std::string(sqlite3_column_int(_stmt, 4)>0 ? "+" : "-");
        _prev_start = bed.start;
        _status = BED_VALID;
        return true;
    default:
        std::cerr << "Error: " << rc << ": " << sqlite3_errmsg(_db) << "\n";
        break;
    }
    return false;
}


