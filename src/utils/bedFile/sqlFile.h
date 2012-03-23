#ifndef SQLFILE_H
#define SQLFILE_H

#include <sqlite3.h> 
#include "bedFile.h"

class SqlFile : public BedFile {

public:
    bool GetNextBed(BED &bed, bool forceSorted = false);
    void Open(void);
    void Close(void);
    template <typename T> inline void SqlReportBed(const T &bed, 
                                                   CHRPOS start, CHRPOS end, const char* finalize) {
        /****   TODO: fields business ****/
        if (start<0) start = bed.start;
        if (end<0) end = bed.end;
        printf ("%s\t%d\t%d\t%s\t%s%s", 
                bed.chrom.c_str(), start, end, bed.name.c_str(),
                bed.score.c_str(), finalize);
    }

private: 
    sqlite3 *_db;
    sqlite3_stmt *_stmt;

};

#endif /* SQLFILE_H */
