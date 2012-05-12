/*****************************************************************************
  toSqlMain.cpp

  (c) 2012 - Jacques Rougemont, EPFL

  Licenced under the GNU General Public License 2.0 license.
******************************************************************************/
#include "toSql.h"
#include "version.h"

#define PROGRAM_NAME "bedtools tosql"

#define PARAMETER_CHECK(param, paramLen, actualLen) (strncmp(argv[i], param, min(actualLen, paramLen))== 0) && (actualLen == paramLen)

void tosql_help(void);

int tosql_main(int argc, char **argv) {
    std::string bedFile;
    std::string genomeFile;
    std::string sqlFile;
    bool haveBed = false;
    bool haveGenome = false;
    bool haveSql = false;
    bool showHelp = false;
    for ( int i=1; i<argc; i++ ) {
        int parameterLength = (int)strlen(argv[i]);
        if (PARAMETER_CHECK("-a", 2, parameterLength)) {
	    i++;
            if (i < argc) {
                haveBed = true;
                bedFile = std::string(argv[i]);
            }
        } else if (PARAMETER_CHECK("-g", 2, parameterLength)) {
	    i++;
            if (i < argc) {
                haveGenome = true;
                genomeFile = std::string(argv[i]);
            } 
        } else if (PARAMETER_CHECK("-o", 2, parameterLength)) {
	    i++;
            if (i < argc) {
                haveSql = true;
                sqlFile = std::string(argv[i]);
            } 
        } else {
	    std::cerr << "\n*****ERROR: Unrecognized parameter: " << argv[i] 
		      << " *****\n\n";
            showHelp = true;
        }

    }
    if (!(haveBed && haveGenome && haveSql)) {
        std::cerr << "\n*****\n*****ERROR: Need -a and -b files. \n*****\n";
        showHelp = true;
    }
    if (showHelp) {
        tosql_help();
        return 0;
    } else {
        ToSql *tosql = new ToSql(bedFile, genomeFile, sqlFile);
        delete tosql;
        return 0;
    }
}

void tosql_help(void) {
    std::cerr << "\nTool:    bedtools toSql (aka bedToSql)\n"
	      << "Version: " << VERSION << "\n"
	      << "Summary: Convert bedfile to sqlite file.\n\n"
	      << "Usage:   " << PROGRAM_NAME << " [OPTIONS] -a <bed/gff/vcf> \n\n"
	      << "Options: \n"
	      << "\t-f\t" << "Field names (comma.separated list).\n\n";
    exit(1);

}
