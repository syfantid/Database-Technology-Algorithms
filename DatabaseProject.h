#ifndef DATABASE_PROJECT_H
#define DATABASE_PROJECT_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "dbtproj.h"

bool operator > (const record_t& a, const record_t& b);

bool operator < (const record_t& a, const record_t& b);

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
