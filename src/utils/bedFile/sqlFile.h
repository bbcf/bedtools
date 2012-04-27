#ifndef SQLFILE_H
#define SQLFILE_H

#include <sqlite3.h> 

class SqlFile : public BedFile {

public:
    bool GetNextBed(BED &bed, bool forceSorted = false);
    void Open(void);
    void Close(void);
    template <typename T> inline void SqlReportBed(const T &bed, 
                                                   CHRPOS start, CHRPOS end, const char* finalize) {
        if (start<0) start = bed.start;
        if (end<0) end = bed.end;
        printf ("%s\t%d\t%d\t%s", 
                bed.chrom.c_str(), start, end, bed.name.c_str());
        if (bed.score.size()) printf ("\t%s", bed.score.c_str());
        if (bed.strand.size()) printf ("\t%s", bed.score.c_str());
        for ( std::vector<std::string>::const_iterator othIt = bed.fields.begin();
              othIt != bed.fields.end(); othIt++ ) {
            printf("%s\t", othIt->c_str());
        }
        printf ("%s", finalize);
    }

private: 
    sqlite3 *_db;
    sqlite3_stmt *_stmt;
    std::map< std::string, long > _chrom_table;
    std::map< std::string, long >::const_iterator I_chrom;

    inline void prepareNextChrom( const std::string& );

};

#endif /* SQLFILE_H */
