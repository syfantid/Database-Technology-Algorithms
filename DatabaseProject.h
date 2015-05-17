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

bool readFileBlock(int number, int b, std::vector<std::string>& names,
                   std::vector<FILE*>& files,block_t *buffer);

#endif // DATABASE_PROJECT_H
