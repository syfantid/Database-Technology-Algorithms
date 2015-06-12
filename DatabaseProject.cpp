#include "DatabaseProject.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <math.h>

using namespace std;

struct CompareRecord : public std::binary_function<record_t*, record_t*, bool>
{
    char field;
    CompareRecord(char field = 0): field(field) {}
    //CompareRecord(unsigned char field) { this->type = field; }
    bool operator()(const record_t* lhs, const record_t* rhs) const
    {
        if(field == '0') { //Compare based on record ID
            return lhs->recid > rhs->recid;
        } else if(field == '1') { //Compare based on record number
            return lhs->num >rhs->num;
        } else if(field == '2') { //Compare based on record string
            return (strcmp(lhs->str,rhs->str) > 0);
        } else if(field == '3') { //Compare based on record string and number
            if (lhs->num < rhs->num) {
                return false;
            } else if (lhs->num > rhs->num) {
                return true;
            } else {
                return (strcmp(lhs->str,rhs->str) > 0);
            }
        } else {
            cout<<"Wrong field! Please give a field between 0 and 3!"<<endl;
            exit(0);
        }
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
	//2 dummy variables just to call merge sort
	unsigned int segmentsNumber;
	unsigned int sortingPhases;

	unsigned int IOsNumber; //#of IOs performed including these of merge sort
    char *outofMerge= new char[30];
    MergeSort(infile,field,buffer,nmem_blocks,outofMerge,&segmentsNumber,&sortingPhases,
              &IOsNumber);

    //Allocate memory to buffer; We need 2 blocks, 1 for input and 1 for output
    buffer = (block_t *) malloc (sizeof(block_t)*2);
    //Open file for reading
    FILE *inputfile, *outputfile;
    inputfile = fopen(outofMerge,"rb"); //Open the sorted file for reading
    outputfile = fopen(outfile,"wb"); //Open a file for output
    //Always keeping the previous record we read
    record_t previous;
    unsigned outindex = 0; //The index for the output buffer
    unsigned uniques = 0; //Number of unique records

    cout<<"Eliminating Duplicates..."<<endl;
    while(!feof(inputfile)) { //while there are more blocks in the file
        //Read one sorted block at a time
        fread(&buffer[0], sizeof(block_t), 1, inputfile);
        if(buffer[0].nreserved == 0) { //Because FEOF is set AFTER the end of file
            break;
        }
        for(unsigned i=0; i<buffer[0].nreserved; ++i) { //for all block entries
            //printRecord(buffer[0].entries[i]);
            if(!(buffer[0].entries[i] == previous)) { //It's not a duplicate
                //cout<<"-----------------------UNIQUE!"<<endl;
                uniques += 1;
                //printRecord(buffer[0].entries[i]);
                if(outindex == MAX_RECORDS_PER_BLOCK) {
                    //If output buffer is full, append it to file
                    fwrite(&buffer[1], sizeof(block_t), 1, outputfile);
                    IOsNumber+=1;
                    //Empty the output buffer block
                    memset(&buffer[1],0,sizeof(block_t));
                    outindex = 0;
                }
                memcpy(&buffer[1].entries[outindex], &buffer[0].entries[i],
                       sizeof(record_t));
                ++buffer[1].nreserved;
                ++outindex;
            }
            previous = buffer[0].entries[i]; //Update previous record
        }
    }
    //Close files
    fclose(inputfile);
    fclose(outputfile);
    //Free buffer memory
    free(buffer);
    *nunique = uniques;
    *nios = IOsNumber;


}

void MergeSort (char *infile, unsigned char field, block_t *buffer,
                unsigned int nmem_blocks, char *outfile,
                unsigned int *nsorted_segs, unsigned int *npasses,
                unsigned int *nios) {
    cout<<"Merge Sorting..."<<endl;
    if(nmem_blocks > 2) {
        //Open the file
        FILE *inputfile,*outputfile;
        inputfile = fopen(infile,"rb");
        //Allocate memory to buffer
        buffer = (block_t *) malloc (sizeof(block_t)*nmem_blocks);
        //Allocate disc space for records in buffer
        record_t *records = (record_t*)malloc(nmem_blocks*MAX_RECORDS_PER_BLOCK*sizeof(record_t));
        int recordsIndex = 0;
        int nreserved;
        unsigned ios = 0;
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
            int recSecIndex=0;
            string name = createFileName(fileNumber);
            outputfile = fopen(name.c_str(), "wb");
            for (unsigned b=0; b<nmem_blocks;b++)
            {
                for (int i=0;i<MAX_RECORDS_PER_BLOCK && recSecIndex<recordsIndex;i++)
                {
                    buffer[b].entries[i]=records[recSecIndex];
                    recSecIndex++;
                }
                fwrite(&buffer[b],sizeof(block_t),1,outputfile);
                ios+=1;
            }
            fclose(outputfile);

            //Emptying buffer
            memset(buffer, 0, nmem_blocks*sizeof(block_t));
            recordsIndex = 0;
        }
        fclose(inputfile); //Closing the initial file
        free(records);
        //N-WAY MERGE - STEP 1...N
        unsigned phase = 1; //Number of phases for the merging
        int filesInPhase = fileNumber;
        int filesProducedInPhase;
        int inputFileNumber = 1; //Current file number e.g.5
        int outputFileNumber = filesInPhase+1; //Next number of name available
        //e.g.segment6.bin
        int initialOutputFileNumber = outputFileNumber;
        do{ //While we have more than 1 file for the next round to merge -
            //cout<<"PHASE "<<phase<<endl;
            filesProducedInPhase = 0;
            //The current n-1 files of a block
            vector<FILE*> currentFiles(nmem_blocks-1);
            //The names of the open currentFiles
            vector<string> names(nmem_blocks-1);
            int filesRead = 0; //Files read so far
            //Sorts the records in the buffer based on the specified field
            CompareRecord cmp(field);
            priority_queue<record_t*, std::vector<record_t*>, CompareRecord> pq(cmp);
            //Keeps the current index of each block (both input and output blocks)
            vector<unsigned> index(nmem_blocks,0);

            while(filesRead < filesInPhase) {
                int filesInRound = 0;
                /*cout<<"Files read in phase so far: "<<filesRead<<endl<<
                "Total files in phase: "<<filesInPhase<<endl;*/
            //While there are more current
            //files to be read in this round - WHILE FOR FILES IN ROUND
                string outputName = createFileName(outputFileNumber);
                outputfile = fopen(outputName.c_str(), "wb");

                for(unsigned b=0; b<nmem_blocks-1; ++b) { //For each n-1 files read
                //their first block into n-1 buffer blocks
                    if(inputFileNumber<initialOutputFileNumber) {
                        names[b] = createFileName(inputFileNumber);
                        currentFiles[b] = fopen(names[b].c_str(),"rb");
                        fread(&buffer[b],sizeof(block_t),1,currentFiles[b]);
                        if(!feof(currentFiles[b])) {
                            ++filesRead; //Each buffer block "reads" a different file
                            ++inputFileNumber; //Update the file number to be read next
                            buffer[b].entries[index[b]].dummy1 = b;
                            //Put a record inside the minheap
                            pq.push(&buffer[b].entries[index[b]]);
                            --buffer[b].dummy;
                            ++filesInRound;
                        }
                    }
                }
                bool flag = true;
                //While there is still at least one buffer block with records
                while(flag) {
                    flag = false;
                    if(buffer[nmem_blocks-1].nreserved == MAX_RECORDS_PER_BLOCK) {
                //If output buffer is full, append it to file
                //Append buffer output block to file. If file doesn't exist open it.
                        buffer[nmem_blocks-1].dummy = MAX_RECORDS_PER_BLOCK;
                        fwrite(&buffer[nmem_blocks-1], sizeof(block_t), 1,
                               outputfile);
                        ios+=1;
                        //Empty the output buffer block
                        memset(&buffer[nmem_blocks-1],0,sizeof(block_t));
                        index[nmem_blocks-1] = 0;
                    }
                    memcpy(&buffer[nmem_blocks-1].entries[index[nmem_blocks-1]],
                        pq.top(),sizeof(record_t));
                    ++buffer[nmem_blocks-1].nreserved;
                    ++index[nmem_blocks-1];
                    int b = ((record_t*)pq.top())->dummy1;
                    pq.pop();
                    ++index[b];
                    if(index[b] == buffer[b].nreserved) { //If the buffer block has
                    //reached its end, read the next block from file
                        if(fread(&buffer[b],sizeof(block_t),1,currentFiles[b])) {
                            index[b] = 0;
                        } //else the file has finished
                    }
                    for(int b=0; b<filesInRound; ++b) {
                        if(buffer[b].dummy != 0) {
                            flag = true;
                        }
                    }
                    if(index[b] < buffer[b].nreserved) {
                        buffer[b].entries[index[b]].dummy1 = b;
                        pq.push(&buffer[b].entries[index[b]]);
                        --buffer[b].dummy;
                    }
                }
                while(!pq.empty()) { //While there are more elements in the minheap
                    if(index[nmem_blocks-1] >= buffer[nmem_blocks-1].nreserved) {
                        buffer[nmem_blocks-1].dummy = buffer[nmem_blocks-1].nreserved;
                        fwrite(&buffer[nmem_blocks-1],sizeof(block_t),1,outputfile);
                        ios+=1;
                        memset(&buffer[nmem_blocks-1],0,sizeof(block_t));
                        index[nmem_blocks-1] = 0;
                    }
                    memcpy(&buffer[nmem_blocks-1].entries[index[nmem_blocks-1]],
                           pq.top(),sizeof(record_t));
                    ++index[nmem_blocks-1];
                    ++buffer[nmem_blocks-1].nreserved;
                    pq.pop();
                }
                buffer[nmem_blocks-1].dummy = buffer[nmem_blocks-1].nreserved;
                fwrite(&buffer[nmem_blocks-1],sizeof(block_t),1,outputfile);
                ios+=1;
                memset(&buffer[nmem_blocks-1],0,sizeof(block_t));
                        index[nmem_blocks-1] = 0;
                ++outputFileNumber;
                ++filesProducedInPhase;
                fclose(outputfile);
                for(unsigned i=0; i<nmem_blocks-1; ++i) { //Closing current files
                    remove(names[i].c_str()); //Deleting current files, so that
                    //the disc isn't full of not needed files
                }
                memset(buffer,0,nmem_blocks*sizeof(block_t));
                for(unsigned b=0; b<nmem_blocks; ++b) {
                    index[b] = 0; //Each block's index returns to 0
                }
            }
            phase+=1;
            fileNumber += filesProducedInPhase;
            outputFileNumber = inputFileNumber+filesProducedInPhase;
            initialOutputFileNumber = outputFileNumber;
            filesInPhase = filesProducedInPhase;
        } while(filesProducedInPhase!=1);
        free(buffer);
        *npasses = phase;
        *nsorted_segs = fileNumber;
        *nios = ios;
        string str = createFileName(fileNumber);
        strcpy(outfile, str.c_str());
        //free(records);
    } else {
        cout<<"The buffer size is too small!"<<endl;
        exit(0);
    }
}
void MergeJoin (char *infile1, char *infile2, unsigned char field, block_t *buffer, unsigned int nmem_blocks, char *outfile, unsigned int *nres, unsigned int *nios)
{
    char outfile_R[]= "1outfile.bin";
    char outfile_S[]= "2outfile.bin";

    unsigned int nunique_R;
    unsigned int nunique_S;
    unsigned int niosD_R;
    unsigned int niosD_S;
    int numberofIOS=0;
    //MergeSort(infile1,field,buffer,nmem_blocks,mergeOutFile_R,&nsorted_segs_R,&npasses_R,&nios_R);
    //MergeSort(infile2,field,buffer,nmem_blocks,mergeOutFile_S,&nsorted_segs_S,&npasses_S,&nios_S);
    EliminateDuplicates(infile1,field,buffer,nmem_blocks,outfile_R,&nunique_R,&niosD_R);
    EliminateDuplicates(infile2,field,buffer,nmem_blocks,outfile_S,&nunique_S,&niosD_S);
    numberofIOS=numberofIOS+niosD_R+niosD_S;
    FILE *inputFile_R, *inputFile_S,*outputFile;
    inputFile_R= fopen(outfile_R,"r");
    cout<<outfile_R<<endl;
    inputFile_S= fopen(outfile_S,"r");
    outputFile=fopen(outfile,"wb");
    buffer = (block_t *) malloc (sizeof(block_t)*nmem_blocks);
    fread(&buffer[0],sizeof(block_t),1,inputFile_R);
    fread(&buffer[1],sizeof(block_t),1,inputFile_S);
    numberofIOS= numberofIOS+2;
    unsigned int indexR=0;
    unsigned int indexS=0;
    unsigned int bufOutIndex=2;//the first 2 buffers are for the input files
    int bufOutEntrIndex=0;
    int result=0;
    unsigned int blockID=0;
    int counter=0;
    while(!feof(inputFile_R))//end of input files
    {

        if(bufOutIndex<nmem_blocks)//end of out buffers
        {

            if(indexR<=buffer[0].nreserved-1)//index of first file smaller than entries
            {
                if (indexS<=buffer[1].nreserved-1)//index of second file
                {
                    if(field=='0')//join with ID
                    {

                        result= compareID(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);

                    }
                    else if(field=='1')//join with num
                    {
                        result= compareNUM(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);

                    }
                    else if(field=='2')//join with STR
                    {
                        result= compareSTR(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);
                    }
                    else if(field=='3')//join with num and STR
                    {
                        result= compareNUMSTR(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);
                    }

                    //cout<<result<<endl;
                    if (bufOutEntrIndex== MAX_RECORDS_PER_BLOCK)//if out buffer is full
                    {
                        //cout<<"5"<<endl;
                        if(bufOutIndex<nmem_blocks-1)//if there is available outbuffer index
                        {
                            //cout<<"1"<<endl;
                            bufOutIndex++;
                            bufOutEntrIndex=0;

                        }
                        else//if not
                        {
                            for(unsigned int k=2;k<nmem_blocks;k++)//write the buffers to file
                            {
                                //cout<<"2"<<endl;
                                buffer[k].blockid=blockID;
                                blockID++;
                                buffer[k].valid=true;
                                buffer[k].nreserved= MAX_RECORDS_PER_BLOCK;
                                fwrite(&buffer[k],sizeof(block_t),1,outputFile);
                                numberofIOS++;//each writing
                                memset(&buffer[k],0,sizeof(block_t));//flush the buffer
                            }
                            bufOutIndex=2;
                            bufOutEntrIndex=0;

                        }
                    }
                    else
                    {
                        if (result==-1)//R file's record is smaller than S file's record (sorted with mergesort);
                        {
                            indexR++;//next record of R file
                            //cout<<"-1"<<endl;
                        }
                        else if(result==1)//S file's record is smaller than R file's record
                        {
                            //cout<<"1"<<endl;
                            indexS++;//next record of S file
                        }
                        else
                        {

                             memcpy(&buffer[bufOutIndex].entries[bufOutEntrIndex],&buffer[0].entries[indexR],sizeof(record_t));//join random (here with the R record)
                             //printRecord(buffer[bufOutIndex].entries[bufOutEntrIndex]);
                             //cout<<"l"<<endl;
                             indexR++;
                             indexS++;
                             bufOutEntrIndex++;
                             counter++;


                        }
                    }
                }
                else
                {
                    memset(&buffer[1],0,sizeof(block_t));
                    fread(&buffer[1],sizeof(block_t),1,inputFile_S);
                    numberofIOS++; //reading
                    if (buffer[1].nreserved==0)
                    {
                        break;
                    }
                    indexS=0;
                }
            }
            else
            {
                memset(&buffer[0],0,sizeof(block_t));
                fread(&buffer[0],sizeof(block_t),1,inputFile_R);
                numberofIOS++; //reading
                if (buffer[0].nreserved==0)
                {
                    break;
                }
                indexR=0;
            }
        }
    }
    for(unsigned int k=2;k<nmem_blocks;k++)//write the remaining buffers to file
    {
        if(buffer[k].nreserved!=0)
        {
            buffer[k].blockid=blockID;
            blockID++;
            buffer[k].valid=true;
            buffer[k].nreserved= MAX_RECORDS_PER_BLOCK;
            fwrite(&buffer[k],sizeof(block_t),1,outputFile);
            numberofIOS++; //writing
            memset(&buffer[k],0,sizeof(block_t));//flush the buffer
        }
    }
    free(buffer);
    fclose(inputFile_R);
    fclose(inputFile_S);
    fclose(outputFile);
    *nres=counter;
    *nios=numberofIOS;

    }





void printRecord(record_t r) {
    printf("This is record id: %-5d, num: %-5d, str: %s\n",r.recid,r.num,r.str);
}

string createFileName(unsigned fileNumber) {
    stringstream ss;
    ss << "segment" << fileNumber << ".bin";
    return ss.str();
}

bool operator == (const record_t& a, const record_t& b) {
    return a.num == b.num
    or     (strcmp(a.str,b.str) == 0);
}


