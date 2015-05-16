#include "DatabaseProject.h"
#include <iostream>
#include "MinHeap.h"
using namespace std;

int compareNUM(const void *p1,const void *p2) {
    record_t *a=(record_t *)p1;
    record_t *b=(record_t *)p2;

    if (a->num < b->num) {
        return -1;
    } else if (a->num > b->num) {
        return 1;
    } else {
        return 0;
    }

}
int compareSTR(const void *p1,const void *p2) {
    record_t *a=(record_t *)p1;
    record_t *b=(record_t *)p2;

    if (strcmp(a->str,b->str)<0) {
        return -1;
    } else if (strcmp(a->str,b->str)>0) {
        return 1;
    } else {
        return 0;
    }

}
int compareID(const void *p1,const void *p2) {
    record_t *a=(record_t *)p1;
    record_t *b=(record_t *)p2;

    if (a->recid < b->recid) {
        return -1;
    } else if (a->recid > b->recid) {
        return 1;
    } else {
        return 0;
    }
}
int compareNUMSTR(const void *p1,const void *p2) {
    record_t *a=(record_t *)p1;
    record_t *b=(record_t *)p2;
    if (a->num < b->num) {
        return -1;
    } else if (a->num > b->num) {
        return 1;
    } else {
        return compareSTR(p1,p2);
    }

}

void EliminateDuplicates (char *infile, unsigned char field, block_t *buffer,
                          unsigned int nmem_blocks, char *outfile,
                          unsigned int *nunique, unsigned int *nios) {

}
void MergeSort (char *infile, unsigned char field, block_t *buffer,
                unsigned int nmem_blocks, char *outfile,
                unsigned int *nsorted_segs, unsigned int *npasses,
                unsigned int *nios) {
    FILE *inputfile;
    inputfile=fopen(infile,"r");
    buffer= (block_t *) malloc (sizeof(block_t)*nmem_blocks);
    record_t records[nmem_blocks*MAX_RECORDS_PER_BLOCK];
    int recordsIndex=0;
    int nreserved;
    bool flag = false;
    //MinHeap<record_t*> minheap;

    while(!feof(inputfile)) {
        fread(buffer, sizeof(block_t), nmem_blocks, inputfile);

        for (unsigned b=0; b<nmem_blocks; b++) {

            nreserved = buffer[b].nreserved;

            for (int i=0; i<nreserved; ++i) {
                records[recordsIndex]=buffer[b].entries[i];
                recordsIndex++;
            }


        }
        qsort(records,recordsIndex, sizeof(record_t), compareNUM);
        for (int i=0; i<recordsIndex; i++) {

            printf("this is record id: %d, num: %d, str: %s\n",records[i].recid,
                   records[i].num,records[i].str);

        }
        /* memset = std::fill
           memcpy = std::copy
           qsort = std::sort
           strcmp = std::string::operator==
           */
        memset(buffer, 0, nmem_blocks*sizeof(block_t));
        recordsIndex=0;

    }
    fclose(inputfile);
    cout<<"hello"<<endl;






}

bool operator > (const record_t& a, const record_t& b) {
    if (a.num> b.num)
        return true;
    return false;
}
bool operator < (const record_t& a, const record_t& b) {
    if (a.num < b.num)
        return true;
    return false;
}

