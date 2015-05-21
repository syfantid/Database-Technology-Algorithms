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
        //Files contain records, not whole blocks
        fclose(outputfile);

        //Emptying buffer
        memset(buffer, 0, nmem_blocks*sizeof(block_t));
        recordsIndex = 0;
    }
    fclose(inputfile); //Closing the initial file
    //N-WAY MERGE - STEP 1...N
    unsigned phase = 1; //Number of phases for the merging
    int filesInPhase = fileNumber;
    int filesProducedInPhase;
    int inputFileNumber = 1; //Current file number e.g.5
    int outputFileNumber = filesInPhase+1; //Next number of name available
    //e.g.segment6.bin
    int initialOutputFileNumber = outputFileNumber;
    do{ //While we have more than 1 file for the next round to merge -
        int nWrites = 0; //Number of writings to file
        cout<<"Phase "<<phase<<endl;
        filesProducedInPhase = 0;
        //The current n-1 files of a block
        vector<FILE*> currentFiles(nmem_blocks-1);
        //The names of the open currentFiles
        vector<string> names(nmem_blocks-1);
        int filesRead = 0; //Files read so far
        priority_queue<record_t*,vector<record_t*>, CompareRecordNum > pq;
        //Keeps the current index of each block (both input and output blocks)
        vector<unsigned> index(nmem_blocks,0);

        while(filesRead < filesInPhase) {
            int fileWrites = 0;
            cout<<"Files read in phase so far: "<<filesRead<<endl<<
            "Total files in phase: "<<filesInPhase<<endl;
        //While there are more current
        //files to be read in this round - WHILE FOR FILES IN ROUND
            string outputName = createFileName(outputFileNumber);
            outputfile = fopen(outputName.c_str(), "wb");
            for(unsigned b=0; b<nmem_blocks-1; ++b) { //For each n-1 files read
            //their first block into n-1 buffer blocks
                if(inputFileNumber<initialOutputFileNumber &&
                   readFileBlock(inputFileNumber,b,names,currentFiles,buffer)) {
                    ++filesRead; //Each buffer block "reads" a different file
                    ++inputFileNumber; //Update the file number to be read next
                    buffer[b].entries[index[b]].blockID = b;
                    //Put a record inside the minheap
                    pq.push(&buffer[b].entries[index[b]]);
                }
            }
            bool flag = true;
            //While there is still at least one buffer block with records
            while(flag) {
                flag = false;
                if(buffer[nmem_blocks-1].nreserved == MAX_RECORDS_PER_BLOCK) {
            //If output buffer is full, append it to file
            //Append buffer output block to file. If file doesn't exist open it.
                    cout<<"Buffer output block maxed out!"<<endl;
                    /*for(int i=0; i<buffer[nmem_blocks-1].nreserved; ++i) {
                        printRecord(buffer[nmem_blocks].entries[i]);
                    }*/
                    fwrite(buffer[nmem_blocks-1].entries, MAX_RECORDS_PER_BLOCK,
                            sizeof(record_t), outputfile);
                    fileWrites += MAX_RECORDS_PER_BLOCK;
                    cout<<"So far we've written: "<<fileWrites<<" records."<<endl;
                    //Empty the output buffer block
                    memset(&buffer[nmem_blocks-1],0,sizeof(block_t));
                    index[nmem_blocks-1] = 0;
                }
                //Store the minimum record to buffer output block
                memcpy(&buffer[nmem_blocks-1].entries[index[nmem_blocks-1]],
                       pq.top(),sizeof(record_t));

                //printRecord(*pq.top());
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
                    --buffer[b].dummy;
                }
                for(unsigned b=0; b<nmem_blocks-1; ++b) {
                    if(buffer[b].dummy != 0) {
                        flag = true;
                    }
                }
            }
            while(!pq.empty()) { //While there are more elements in the minheap
                //cout<<"Remaining queue"<<endl;
                //printRecord(*pq.top());
                fwrite(pq.top(),1,sizeof(record_t),outputfile);
                //printRecord(*pq.top());
                ++fileWrites;
                pq.pop();
            }
            cout<<"Total number of writings to file "<<outputFileNumber<<" is "
                <<fileWrites<<endl;
            nWrites += fileWrites;
            ++outputFileNumber;
            ++filesProducedInPhase;
            fclose(outputfile);
            for(unsigned i=0; i<nmem_blocks-1; ++i) { //Closing current files
                fclose(currentFiles[i]);
            }
            memset(buffer,0,nmem_blocks*sizeof(block_t));
            for(unsigned b=0; b<nmem_blocks; ++b) {
                index[b] = 0; //Each block's index returns to 0
            }
        }
        cout<<"Total number of writings in phase (should be 200): "
            <<nWrites<<endl;
        phase+=1;
        fileNumber += filesProducedInPhase;
        outputFileNumber = inputFileNumber+filesProducedInPhase;
        initialOutputFileNumber = outputFileNumber;
        filesInPhase = filesProducedInPhase;
    } while(filesProducedInPhase!=1);
    free(buffer);
    phase-=1;
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
    buffer[b].dummy = 0;
    bool flag = false;
    //Read all the records from the file's block to memory
    for(unsigned rec=0; rec<MAX_RECORDS_PER_BLOCK; rec++) {
        if(fread(&buffer[b].entries[rec],sizeof(record_t),1,currentFiles[b]) == 1) {
            buffer[b].nreserved += 1;
            buffer[b].dummy+=1;
            //printRecord(buffer[b].entries[rec]);
            flag = true; //We've read at least one record
        } else {
            if(!flag)
                return false;
        }
    }
    return true;
}

bool readFileBlock(int number, unsigned b,vector<string>& names,vector<FILE*>&
                   currentFiles,block_t *buffer) {
    names[b] = createFileName(number);
    if(!(currentFiles[b] = fopen(names[b].c_str(),"rb"))) {
        return false;
    }

    cout<<"File to be read "<<names[b]<<endl;
    //Read all the records from the file's block to memory
    return readBlock(b,currentFiles,buffer);
}

bool readOpenFileBlock(unsigned b, vector<FILE*>& currentFiles,block_t *buffer,
                       vector<string>& names) {
    cout<<"File to be read "<<names[b]<<endl;
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

