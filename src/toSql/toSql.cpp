/*****************************************************************************
  toSql.cpp

  (c) 2012 - Jacques Rougemont, EPFL

  Licenced under the GNU General Public License 2.0 license.
******************************************************************************/
#include "toSql.h"
#include <genomeFile.h>
#include <stdio.h>

ToSql::ToSql( std::string bedFile, std::string genomeFile, std::string sqlFile )
    : _last_chrom(), _bedFile(bedFile), _genomeFile(genomeFile), _sqlFile(sqlFile)
{
    Open();
    BedFile *_bed = new BedFile(_bedFile);
    GenomeFile *_genome = new GenomeFile(_genomeFile);
    std::vector< std::string > chromList = _genome->getChromList();
    for ( std::vector< std::string >::const_iterator Ic = chromList.begin();
	  Ic != chromList.end();
	  Ic++ ) {
	_chrom_table[*Ic] = _genome->getChromSize(*Ic);
    }
    delete _genome;
    BED a;
    if (!prepareTables()) {
	Close();
	return;
    }
    while (_bed->GetNextBed(a)) {
	if (!insertBed(a)) {
	    _bed->Close();
	    Close();
	    return;
	}
    }
    _bed->Close();
}

ToSql::~ToSql(void) {
    Close();
}

inline void ToSql::Open(void) {
    sqlite3_open_v2( _sqlFile.c_str(), &_db, 
                     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
}

void ToSql::Close(void) {
    _chrom_table.clear();
    sqlite3_finalize(_stmt);
    sqlite3_close(_db);
}


bool ToSql::prepareTables() {
    std::string sql_exec;
    const char *_dummy;
    sql_exec = std::string("CREATE TABLE IF NOT EXISTS 'attributes' ('key' TEXT, 'value' TEXT)"); 
    if ( sqlite3_exec( _db, sql_exec.c_str(), NULL, 0, &_sqlErrMsg ) ) {
	std::cerr << "Create table error: " << _sqlErrMsg << "\n";
	sqlite3_free(_sqlErrMsg);
	return false;
    }
    sql_exec = std::string("CREATE TABLE IF NOT EXISTS 'chrNames' ('name' TEXT, 'length' INTEGER)");
    if ( sqlite3_exec( _db, sql_exec.c_str(), NULL, 0, &_sqlErrMsg ) ) {
	std::cerr << "Create table error: " << _sqlErrMsg << "\n";
	sqlite3_free(_sqlErrMsg);
	return false;
    }
    sql_exec = std::string("INSERT INTO 'chrNames' (name,length) VALUES (?,?)");
    sqlite3_exec( _db, "begin transaction", NULL, NULL, NULL );
    if ( sqlite3_prepare_v2( _db, sql_exec.c_str(), 2048, &_stmt, &_dummy ) ) {
	std::cerr << "Prepare error: " << sqlite3_errmsg(_db) << "\n";
	return false;
    }
    for ( I_chrom = _chrom_table.begin(); I_chrom != _chrom_table.end(); I_chrom++ ) {
	sqlite3_bind_text( _stmt, 1, I_chrom->first.c_str(), I_chrom->first.size(), NULL );
	sqlite3_bind_int( _stmt, 2, I_chrom->second );
	sqlite3_step( _stmt );
	sqlite3_reset( _stmt );
    }
    sqlite3_finalize( _stmt );
    sqlite3_exec( _db, "commit transaction", NULL, NULL, NULL );
    for ( I_chrom = _chrom_table.begin(); I_chrom != _chrom_table.end(); I_chrom++ ) {
	std::string sql_exec = std::string("CREATE TABLE IF NOT EXISTS '")
	    +I_chrom->first+
	    std::string("' (start INTEGER, end INTEGER, score REAL)");
	if ( sqlite3_exec( _db, sql_exec.c_str(), NULL, 0, &_sqlErrMsg ) ) {
	    std::cerr << "Create table error: " << _sqlErrMsg << "\n";
	    sqlite3_free(_sqlErrMsg);
	    return false;
	}
    }
    return true;
}

bool ToSql::insertBed( const BED &bed ) {
    
    if (bed.chrom == _last_chrom) {
	sqlite3_bind_int( _stmt, 1, bed.start );
	sqlite3_bind_int( _stmt, 2, bed.end );
	sqlite3_bind_double( _stmt, 3, atof(bed.score.c_str()) );
	sqlite3_step( _stmt );
	sqlite3_reset( _stmt );
    } else {
	if (_last_chrom.size()) {
	    sqlite3_finalize( _stmt );
	    sqlite3_exec( _db, "commit transaction", NULL, NULL, NULL );
	}
	_last_chrom = bed.chrom;
	std::string sql_exec = std::string("INSERT INTO '"+_last_chrom+"' (start,end,score) VALUES (?,?,?)");
	const char *_dummy;
	sqlite3_exec( _db, "begin transaction", NULL, NULL, NULL );
	if ( sqlite3_prepare_v2( _db, sql_exec.c_str(), 2048, &_stmt, &_dummy ) ) {
	    std::cerr << "Prepare error: " << sqlite3_errmsg(_db) << "\n";
	    return false;
	}
	return insertBed(bed);
    }
    return true;
}
