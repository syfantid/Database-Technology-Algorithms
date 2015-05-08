#include "dbproj.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;

void EliminateDuplicates (char *infile, unsigned char field, block_t *buffer, unsigned int nmem_blocks, char *outfile,
                    unsigned int *nunique, unsigned int *nios) {

}
void MergeSort (char *infile, unsigned char field, block_t *buffer, unsigned int nmem_blocks, char *outfile,
                    unsigned int *nsorted_segs, unsigned int *npasses, unsigned int *nios){
    FILE *inputfile;
    inputfile=fopen(infile,"r");
    buffer= new block_t[nmem_blocks];
    int nreserved;
    bool flag = false;
    //while (!feof(inputfile)) {

    while(!feof(inputfile)) {

        for (int b=0;b<nmem_blocks;b++) {
            fread(&buffer[b], sizeof(block_t), 1, inputfile);
            cout<<"number"<<b<<endl;
            nreserved = buffer[b].nreserved;

            for (int i=0; i<nreserved; ++i) {
                printf("this is block id: %d, record id: %d, num: %d, str: %s\n",
                        buffer[b].blockid, buffer[b].entries[i].recid, buffer[b].entries[i].num,
                                                                            buffer[b].entries[i].str);
            }

        }
        memset(buffer,NULL,nmem_blocks*sizeof(block_t));


    }





}
