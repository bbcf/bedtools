/*****************************************************************************
  toSql.cpp

  (c) 2012 - Jacques Rougemont, EPFL

  Licenced under the GNU General Public License 2.0 license.
******************************************************************************/
#include "toSql.h"
#include <genomeFile.h>
#include <lineFileUtilities.h>
#include <stdio.h>
#include <sstream>

static const std::map< const std::string, std::string > _sql_types({
        {"start", "INTEGER"},
        {"end", "INTEGER"},
        {"name", "TEXT"},
        {"score", "REAL"},
        {"strand", "INTEGER"}
    });

ToSql::ToSql( std::string bedFile, std::string genomeFile, std::string sqlFile,
	      std::string all_fields, std::string datatype )
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
    Tokenize(all_fields,_fields,',');
    if (!prepareTables(datatype)) {
	Close();
	return;
    }
    BED a;
    _bed->Open();
    while (_bed->GetNextBed(a)) {
        if (!(_bed->_status == BED_VALID && insertBed(a)))  break;
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
    sqlite3_exec( _db, "commit transaction", NULL, NULL, NULL );
    sqlite3_close(_db);
}


bool ToSql::prepareTables( const std::string &datatype ) {
    std::stringstream sql_exec;
    const char *_dummy;
    sql_exec << "CREATE TABLE IF NOT EXISTS 'attributes' ('key' TEXT, 'value' TEXT)"; 
    if ( sqlite3_exec( _db, sql_exec.str().c_str(), NULL, 0, &_sqlErrMsg ) ) {
	std::cerr << "Create table error: " 
                  << sql_exec.str()
                  << _sqlErrMsg << "\n";
	sqlite3_free(_sqlErrMsg);
	return false;
    }
    sql_exec.str("");
    sql_exec << "INSERT INTO 'attributes' (key,value) VALUES ('datatype','"
	     << datatype << "')";
    if ( sqlite3_exec( _db, sql_exec.str().c_str(), NULL, 0, &_sqlErrMsg ) ) {
	std::cerr << "Insert error: " 
                  << sql_exec.str()
                  << sqlite3_errmsg(_db) << "\n";
	return false;
    }
    sql_exec.str("");
    sql_exec << "CREATE TABLE IF NOT EXISTS 'chrNames' ('name' TEXT, 'length' INTEGER)";
    if ( sqlite3_exec( _db, sql_exec.str().c_str(), NULL, 0, &_sqlErrMsg ) ) {
	std::cerr << "Create table error: " 
                  << sql_exec.str()
                  << _sqlErrMsg << "\n";
	sqlite3_free(_sqlErrMsg);
	return false;
    }
    sql_exec.str("");
    sql_exec << "INSERT INTO 'chrNames' (name,length) VALUES (?,?)";
    sqlite3_exec( _db, "begin transaction", NULL, NULL, NULL );
    if ( sqlite3_prepare_v2( _db, sql_exec.str().c_str(), 2048, &_stmt, &_dummy ) ) {
	std::cerr << "Prepare error: " 
                  << sql_exec.str()
                  << sqlite3_errmsg(_db) << "\n";
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
	sql_exec.str("");
	sql_exec << "CREATE TABLE IF NOT EXISTS '" << I_chrom->first << "' (";
	std::vector< std::string >::const_iterator If = _fields.begin();
	sql_exec << *If << " " << _sql_types.find(*If)->second;	
	for ( If++; If != _fields.end(); If++ ) 
	    sql_exec << "," << *If << " " << _sql_types.find(*If)->second;
	sql_exec << ")";
	if ( sqlite3_exec( _db, sql_exec.str().c_str(), NULL, 0, &_sqlErrMsg ) ) {
	    std::cerr << "Create table error: " 
                      << sql_exec.str()
                      << _sqlErrMsg << "\n";
	    sqlite3_free(_sqlErrMsg);
	    return false;
	}
    }
    return true;
}

bool ToSql::insertBed( const BED &bed ) {
    if (bed.chrom == _last_chrom) {
	int fnn = 0;
	for ( size_t fn = 0; fn < _fields.size(); fn++ )    
	    if (std::string("start").compare(_fields[fn]) == 0)
		sqlite3_bind_int( _stmt, fn+1, bed.start );
	    else if (std::string("end").compare(_fields[fn]) == 0)
		sqlite3_bind_int( _stmt, fn+1, bed.end );
	    else if (std::string("name").compare(_fields[fn]) == 0)
		sqlite3_bind_text( _stmt, fn+1, bed.name.c_str(), bed.name.size(), NULL );
	    else if (std::string("score").compare(_fields[fn]) == 0)
		sqlite3_bind_double( _stmt, fn+1, atof(bed.score.c_str()) );
	    else if (std::string("strand").compare(_fields[fn]) == 0)
		sqlite3_bind_int( _stmt, fn+1, bed.strand.compare("-") ? -1 : 1 );
	    else {
		std::string bf =  bed.fields[fnn];
		sqlite3_bind_text( _stmt, fn+1, bf.c_str(), bf.size(), NULL );
		fnn++;
	    }
	sqlite3_step( _stmt );
	sqlite3_reset( _stmt );
    } else {
	if (_last_chrom.size()) {
	    sqlite3_finalize( _stmt );
	    sqlite3_exec( _db, "commit transaction", NULL, NULL, NULL );
	}
	_last_chrom = bed.chrom;
	std::stringstream sql_exec;
	sql_exec << "INSERT INTO '" << _last_chrom << "' (";
	std::vector< std::string >::const_iterator If = _fields.begin();
	sql_exec << *If;	
	for ( If++; If != _fields.end(); If++ ) sql_exec << "," << *If;
	sql_exec << ") VALUES (?";
	for ( size_t i = 1; i < _fields.size(); i++ ) sql_exec << ",?";
	sql_exec << ")";
	const char *_dummy;
	sqlite3_exec( _db, "begin transaction", NULL, NULL, NULL );
	if ( sqlite3_prepare_v2( _db, sql_exec.str().c_str(), 2048, &_stmt, &_dummy ) ) {
	    std::cerr << "Prepare error: " 
                      << sql_exec.str()
                      << sqlite3_errmsg(_db) << "\n";
	    return false;
	}
	return insertBed(bed);
    }
    return true;
}
