#include "DatabaseProject.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <math.h>
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
    //Creating the initial files from the input file - STEP 0
    //While there are more records in the file to be read
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
                records[recordsIndex]= buffer[b].entries[i];
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

        fwrite(records, recordsIndex, sizeof(record_t), outputfile);
        fclose(outputfile);

        //Emptying buffer
        memset(buffer, 0, nmem_blocks*sizeof(block_t));
        recordsIndex = 0;
    }
    fclose(inputfile); //Closing the initial file
    //N-WAY MERGE - STEP 1...N
    unsigned phase = 0; //Number of phases for the merging
    int filesInPhase = fileNumber;
    int filesProducedInPhase = 0;
    int inputFileNumber = 1; //Current file number e.g.5
    int outputFileNumber = filesInPhase+1; //Next number of name available e.g.segment6.bin
    int initialOutputFileNumber = outputFileNumber;
    do{ //While we have more than 1 file for the next round to merge -
         //DO WHILE FOR TOTAL ROUNDS
        vector<FILE*> currentFiles(nmem_blocks-1); //The current n-1 files of a block
        vector<string> names(nmem_blocks-1); //The names of the open currentFiles
        int filesRead = 0; //Files read so far
        priority_queue<record_t*,vector<record_t*>, CompareRecordNum > pq;
        //Keeps the index we are in each block, n-1 for input and the nth for output
        vector<unsigned> index(nmem_blocks,0);
        while(filesRead < filesInPhase) {
        //While there are more current
        //files to be read in this round - WHILE FOR FILES IN ROUND
            string outputName = createFileName(outputFileNumber);
            outputfile = fopen(outputName.c_str(), "ab"); //Opens file for appending
            for(unsigned b=0; b<nmem_blocks-1; ++b) { //For each n-1 files read their
                //first block into n-1 buffer blocks
                if(inputFileNumber<initialOutputFileNumber &&
                   readFileBlock(inputFileNumber,b,names,currentFiles,buffer)) {
                    ++filesRead; //Each buffer block "reads" a different file
                    ++inputFileNumber; //Update the file number to be read next
                    buffer[b].entries[index[b]].blockID = b;
                    pq.push(&buffer[b].entries[index[b]]); //Put a record inside the minheap
                }
            }
            bool flag = true;
            while(flag) { //While there is still at least one buffer block with records
                flag = false;
                if(buffer[nmem_blocks-1].nreserved == MAX_RECORDS_PER_BLOCK) {
                //If output buffer is full, append it to file
                    //Append buffer output block to file. If file doesn't exist open it.
                    fwrite(&buffer[nmem_blocks-1].entries,buffer[nmem_blocks-1].
                           nreserved, sizeof(record_t), outputfile);
                    //Empty the output buffer block
                    memset(&buffer[nmem_blocks-1],0,sizeof(block_t));
                    index[nmem_blocks-1] = 0;
                    fclose(outputfile);
                }
                //Store the minimum record to buffer output block
                memcpy(&buffer[nmem_blocks-1].entries[index[nmem_blocks-1]],
                       pq.top(),sizeof(record_t));
                ++buffer[nmem_blocks-1].nreserved;
                ++index[nmem_blocks-1];
                //printRecord(*pq.top());
                int b = ((record_t*)pq.top())->blockID;
                pq.pop();
                ++index[b];
                if(index[b] == buffer[b].nreserved) { //If the buffer block has
                //reached its end, read the next block from file
                    if(readOpenFileBlock(b,currentFiles,buffer,names)) {
                        index[b] = 0;
                    } //else the file has finished
                }
                if(index[b] < buffer[b].nreserved) {
                    buffer[b].entries[index[b]].blockID = b;
                    pq.push(&buffer[b].entries[index[b]]);
                }
                for(unsigned b=0; b<nmem_blocks-1; ++b) {
                    if(buffer[b].nreserved != 0) {
                        flag = true;
                    }
                }
            }
            while(!pq.empty()) {
                fwrite(pq.top(),1,sizeof(record_t),outputfile);
                printRecord(*pq.top());
                pq.pop();
            }

            ++outputFileNumber;
            ++filesProducedInPhase;
            fclose(outputfile);
            for(unsigned i=0; i<nmem_blocks-1; ++i) {
                fclose(currentFiles[i]);
            }
            memset(buffer,0,nmem_blocks*sizeof(block_t));
            for(unsigned b=0; b<nmem_blocks; ++b) {
                index[b] = 0;
            }
        }
        phase+=1;
        fileNumber += filesProducedInPhase;
        outputFileNumber = inputFileNumber+filesProducedInPhase;
        initialOutputFileNumber = outputFileNumber;
        filesInPhase = filesProducedInPhase;
    } while(filesProducedInPhase!=1);
    npasses = &phase;
    nsorted_segs = &fileNumber;
}

void printRecord(record_t r) {
    printf("This is record id: %-5d, num: %-5d, str: %s\n",r.recid,r.num,r.str);
}

bool readBlock(unsigned b,vector<FILE*>& currentFiles,block_t *buffer) {
//File is already open
    //Each buffer is assigned a specific file to process
    buffer[b].nreserved = 0;
    buffer[b].valid = true;
    buffer[b].blockid = b;

    //Read all the records from the file's block to memory
    for(unsigned rec=0; rec<MAX_RECORDS_PER_BLOCK; rec++) { //MAYBE BUG - PROBLEM IF FILE REACHES END?
        if(fread(&buffer[b].entries[rec],sizeof(record_t),1,currentFiles[b]) == 1) {
            buffer[b].nreserved += 1;
            printRecord(buffer[b].entries[rec]);
        } else {
            return false;
        }
    }
    return true;
}

bool readFileBlock(int number, unsigned b,vector<string>& names,vector<FILE*>& currentFiles,block_t *buffer) {
    names[b] = createFileName(number);
    if(!(currentFiles[b] = fopen(names[b].c_str(),"rb"))) {
        return false;
    }

    cout<<"-----------FILE: "<<names[b]<<"-----------"<<endl;
    //Read all the records from the file's block to memory
    return readBlock(b,currentFiles,buffer);
}

bool readOpenFileBlock(unsigned b, vector<FILE*>& currentFiles,block_t *buffer,vector<string>& names) {
    cout<<"-----------FILE: "<<names[b]<<"-----------"<<endl;
    return readBlock(b,currentFiles,buffer);
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

