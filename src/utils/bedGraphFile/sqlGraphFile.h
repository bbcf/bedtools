#ifndef SQLGRAPHFILE_H
#define SQLGRAPHFILE_H

#include <sqlite3.h> 

class SqlGraphFile : public BedGraphFile {

public:
    template <typename T> BedGraphLineStatus GetNextBedGraph (BEDGRAPH<T>&, int&);
    void Open(void);
    void Close(void);

private: 
    sqlite3 *_db;
    sqlite3_stmt *_stmt;
    std::map< std::string, long > _chrom_table;
    std::map< std::string, long >::const_iterator I_chrom;

    inline void prepareNextChrom( const std::string& );

};


inline void SqlGraphFile::prepareNextChrom( const std::string &chrom ) {
    int rc = sqlite3_finalize(_stmt);
    std::string sql_exec = std::string("select start, end, score from '")+chrom+std::string("' order by start, end"); 
    rc = sqlite3_prepare_v2(_db, sql_exec.c_str(), -1, &_stmt, 0);
}

template <typename T>
BedGraphLineStatus SqlGraphFile::GetNextBedGraph( BEDGRAPH<T> &bedgraph, int &lineNum )
{
    int rc = sqlite3_step(_stmt);
    std::string sql_exec;
    std::stringstream str_depth;
    switch( rc ) {
    case SQLITE_DONE:
        I_chrom++;
        if (I_chrom == _chrom_table.end()) {
            return BEDGRAPH_BLANK;
        }
        prepareNextChrom(I_chrom->first);
        return GetNextBedGraph(bedgraph, lineNum);
        break;
    case SQLITE_ROW:
        bedgraph.chrom = I_chrom->first;
        bedgraph.start = (CHRPOS)sqlite3_column_int(_stmt, 0);
        bedgraph.end = (CHRPOS)sqlite3_column_int(_stmt, 1);
        str_depth << ((char*)sqlite3_column_text(_stmt, 2));
        str_depth >> bedgraph.depth;
        lineNum++;
        return BEDGRAPH_VALID;
        break;
    default:
        std::cerr << "Error: " << rc << ": " << sqlite3_errmsg(_db) << "\n";
        break;
    }
    return BEDGRAPH_INVALID;
}
 
#endif /* SQLGRAPHFILE_H */
