#ifndef DATABASE_PROJECT_H
#define DATABASE_PROJECT_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "dbtproj.h"

bool operator > (const record_t& a, const record_t& b);

bool operator < (const record_t& a, const record_t& b);

#endif // DATABASE_PROJECT_H
