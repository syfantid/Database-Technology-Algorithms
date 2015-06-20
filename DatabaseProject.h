#ifndef DATABASE_PROJECT_H
#define DATABASE_PROJECT_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "dbtproj.h"

#if defined(WIN32) || defined(_WIN32)
	#define FILE_READ "rb"
	#define FILE_WRITE "wb"
#else
	#define FILE_READ "r"
	#define FILE_WRITE "w"
#endif

std::string createFileName(unsigned fileNumber);

void printRecord(record_t r);

bool read(int number, unsigned b,std::vector<std::string>& names,std::vector<FILE*>&
          currentFiles,block_t *buffer);
bool readFileBlock(int number, unsigned b, std::vector<std::string>& names,
                   std::vector<FILE*>& files,block_t *buffer);

bool readBlock(unsigned b,std::vector<FILE*>& currentFiles,block_t *buffer);

bool readOpenFileBlock(unsigned b, std::vector<FILE*>&
                       currentFiles,block_t *buffer,std::vector<std::string>& names);

#endif // DATABASE_PROJECT_H
