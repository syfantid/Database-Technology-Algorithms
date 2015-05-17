#include "DatabaseProject.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include "MinHeap.h"
using namespace std;

struct CompareRecordID : public std::binary_function<record_t*, record_t*, bool>
{
  bool operator()(const record_t* lhs, const record_t* rhs) const
  {
     return lhs->recid > rhs->recid;
  }
};

struct CompareRecordNum : public std::binary_function<record_t*, record_t*, bool>
{
  bool operator()(const record_t* lhs, const record_t* rhs) const
  {
     return lhs->num > rhs->num;
  }
};

struct CompareRecordStr : public std::binary_function<record_t*, record_t*, bool>
{
  bool operator()(const record_t* lhs, const record_t* rhs) const
  {
     return strcmp(lhs->str,rhs->str);
  }
};

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
    //Open the file
    FILE *inputfile,*outputfile;
    inputfile = fopen(infile,"rb");
    //Allocate memory to buffer
    buffer = (block_t *) malloc (sizeof(block_t)*nmem_blocks);
    //Allocate disc space for records in buffer
    record_t *records = (record_t*)malloc(nmem_blocks*MAX_RECORDS_PER_BLOCK*sizeof(record_t));

    int recordsIndex = 0;
    int nreserved;
    unsigned fileNumber = 0; //The next number of file to be written on disc

    //While there are more records in the file to be read - STEP 0
    while(!feof(inputfile)) {
        //Read as many blocks as the buffer fits
        fread(buffer, sizeof(block_t), nmem_blocks, inputfile);
        if(buffer[0].nreserved == 0) { //Because FEOF is set AFTER the end of file
            break;
        }
        //For each block
        for (unsigned b=0; b<nmem_blocks; b++) {
            nreserved = buffer[b].nreserved;
            //For each record in the block
            for (int i=0; i<nreserved; ++i) {
                records[recordsIndex]=buffer[b].entries[i];
                printf("This is record id: %-5d, num: %-5d, str: %s\n",
                       records[recordsIndex].recid, records[recordsIndex].num,
                       records[recordsIndex].str);
                recordsIndex++;
            }
        }

        //Sorts the records in the buffer based on the specified field
        if(field == '0')
            qsort(records,recordsIndex, sizeof(record_t), compareID);
        else if(field == '1')
            qsort(records,recordsIndex, sizeof(record_t), compareNUM);
        else if(field == '2')
            qsort(records,recordsIndex, sizeof(record_t), compareSTR);
        else if(field == '3')
            qsort(records,recordsIndex, sizeof(record_t), compareNUMSTR);

        fileNumber += 1; //Each buffer we fill refers to a sorted segment (different file)
        //Write buffer to file
        string name = createFileName(fileNumber);
        outputfile = fopen(name.c_str(), "wb");

        fwrite(records, sizeof(record_t), recordsIndex, outputfile);
        fclose(outputfile);

        //Emptying buffer
        memset(buffer, 0, nmem_blocks*sizeof(block_t));
        recordsIndex = 0;
    }
    fclose(inputfile); //Closing the initial file
    //N-WAY MERGE - STEP 1...N
    vector<FILE*> files(nmem_blocks); //The open n-1 files, one for each buffer slot
    vector<string> names(nmem_blocks); //The names of the open files in each buffer
    unsigned phase = 0; //Number of phases for the merging
    int number = 1; //Current file number
    int filesProducedInPhase = 0;

    while(filesProducedInPhase != 1 && phase != 1) { //PHASE != 1 ONLY FOR TESTING
        //Give n-1 buffer blocks for input; 1 for output

        priority_queue<record_t*,vector<record_t*>, CompareRecordNum > pq;
        vector<unsigned> index(nmem_blocks,0); //Keeps the index we are in each block
        //Put the first block of the first nmem_blocks-1 files into the buffer blocks

        for(unsigned b=0; b<nmem_blocks-1; ++b) {
            readFileBlock(number,b,names,files,buffer);
            ++number;
            pq.push(&buffer[b].entries[index[b]]); //Put a record inside the minheap
        }
            /*bool flag = false;
            if(buffer[b].nreserved!=0) { //If block isn't empty
                if(index[b] >= buffer[b].nreserved) {//If all the buffer block's
                //elements are already in the minheap
                    if(readFileBlock(number,b,names,files,buffer)) { //If there are more blocks in
                    //the file to be read, read one
                        index[b] = 0; //Starts over
                        flag = true;
                    }
                } else { //If there are still elements to be put in the minheap
                    flag = true;
                }
            } else { //If the buffer block is empty - First nmem_blocks-1 iterations will end up here
            //Fill the buffer block with a file block
                if(readFileBlock(number,b,names,files,buffer)) {
                    flag = true;
                }
                ++number;
            }
            if(flag) {
                pq.push(&buffer[b].entries[index[b]]); //Put a record inside the minheap
            }*/
        /*//while(аявеио дем евеи текеиысеи) - EDWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW!!!!!
        //When the buffer is full (n-1 slots) put everything in a minheap
        for(int b=0; b<nmem_blocks-1; ++b) {
            for(i=0; i<buffer[b].nreserved; ++i) {
                pq.push(&buffer[b].entries[i]); //Put all the records from the
                //n-1 blocks inside a minheap
            }
        }
        //Empty the minheap into one file and thus merge the blocks
        //The last block will be filled and written to file n-1 times
        //e.g. The last block fits 10 records and the other 3 30. Thus we need
        //3 iterations to put the 30 records inside the last block
        outputfile = fopen(createFileName(fileNumber).c_str(), "ab");
        for(times=0; times<nmem_blocks-1; ++times) {
        //Go to the last buffer block and pop as many records as possible
            buffer[nmem_blocks-1].nreserved = 0;
            buffer[nmem_blocks-1].valid = true;
            for(i=0; i<MAX_RECORDS_PER_BLOCK; i++) {
                buffer[nmem_blocks-1].entries[i] = *pq.top();
                buffer[nmem_blocks-1].nreserved+=1;
                pq.pop();
            }
            //Now last buffer block is full, write to file
            fwrite(&buffer[nmem_blocks-1], sizeof(block_t), buffer[nmem_blocks].nreserved, outputfile)
        }
        fclose(outputfile);*/

        phase+=1;
        fileNumber += filesProducedInPhase;
    }
    npasses = &phase;
    nsorted_segs = &fileNumber;
}

bool readFileBlock(int number, int b,vector<string>& names,vector<FILE*>& files,block_t *buffer) {
    names[b] = createFileName(number);
    files[b] = fopen(names[b].c_str(),"rb");
    //Each buffer is assigned a specific file to process
    buffer[b].nreserved = 0;
    buffer[b].valid = true;
    buffer[b].blockid = b;

    cout<<"-----------FILE: "<<names[b]<<"-----------"<<endl;
    //Read all the records from the file's block to memory
    for(unsigned rec=0; rec<MAX_RECORDS_PER_BLOCK; rec++) { //MAYBE BUG - PROBLEM IF FILE REACHES END
        fread(&buffer[b].entries[rec],sizeof(record_t),1,files[b]);
        if(!feof(files[b])) {
            buffer[b].nreserved += 1;
        } else {
            return false;
        }
        printf("This is record id: %-5d, num: %-5d, str: %s and is part of block %d\n",
            buffer[b].entries[rec].recid, buffer[b].entries[rec].num,
            buffer[b].entries[rec].str,b);
    }
    return true;
}

string createFileName(unsigned fileNumber) {
    stringstream ss;
    ss << "segment" << fileNumber << ".bin";
    return ss.str();
}

/*bool operator > (const record_t& a, const record_t& b) {
    return (a.num> b.num) ;
}
bool operator < (const record_t& a, const record_t& b) {
    return (a.num < b.num) ;
}*/

