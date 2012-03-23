#ifndef SQLFILE_H
#define SQLFILE_H

#include <sqlite3.h> 
#include "bedFile.h"

class SqlFile : public BedFile {

public:
    bool GetNextBed(BED &bed, bool forceSorted = false);
    void Open(void);
    void Close(void);

private: 
    sqlite3 *_db;
    sqlite3_stmt *_stmt;

};

#endif /* SQLFILE_H */
