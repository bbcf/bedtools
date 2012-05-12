/*****************************************************************************
  toSql.h

  (c) 2012 - Jacques Rougemont, EPFL

  Licenced under the GNU General Public License 2.0 license.
******************************************************************************/
#ifndef TOSQL_H
#define TOSQL_H

#include <string>
#include <sqlite3.h>
#include <bedFile.h>


class ToSql {

public:

    ToSql(std::string, std::string, std::string);
    ~ToSql(void);

private:
    void Open(void);
    void Close(void);
    bool prepareTables();
    bool insertBed( const BED& );

    sqlite3 *_db;
    sqlite3_stmt *_stmt;
    char *_sqlErrMsg;
    std::string _last_chrom;
    std::map< std::string, long > _chrom_table;
    std::map< std::string, long >::const_iterator I_chrom;
    std::string _bedFile;
    std::string _genomeFile;
    std::string _sqlFile;

};

#endif /* TOSQL_H */
