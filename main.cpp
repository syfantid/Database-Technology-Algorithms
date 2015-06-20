#include "DatabaseProject.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cstdio>

using namespace std;

void gen_random_string(char *s, const int len) {
    static const char alpha[] =
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alpha[rand() % (sizeof(alpha) - 1)];
    }
    s[len] = 0;
}

int main(int argc, char** argv) {

    srand(time(0)); //Pseudorandom number
	int nblocks = 6000;	// number of blocks in the file
	if (argc == 2) nblocks = atoi(argv[1]);

	record_t record1;
	record_t record2;

	block_t block1;
	block_t block2;

	unsigned int recid = 0;
	FILE *outfile, *outfile2;
	//----------------------GENERATION OF INITIAL FILES--------------------------
	cout<<"Creating input files..."<<endl;
    outfile = fopen("file.bin", FILE_WRITE);
    outfile2 = fopen("file2.bin", FILE_READ);

	char* s = new char[10];

	for (int b=0; b<nblocks; ++b) { // for each block
		block1.blockid = b;
		block2.blockid = b;
		for (int r=0; r<MAX_RECORDS_PER_BLOCK; ++r) { // for each record
			// prepare a record for file 1
			record1.recid = recid;
			record1.num = rand() % (nblocks*30);
            gen_random_string(s,5);
			strcpy(record1.str,s);//Put a random string to each record

			// prepare a record for file 2
			record2.recid = recid++;
			record2.num = rand() % (nblocks*30);
            gen_random_string(s,5);
			strcpy(record2.str,s);//Put a random string to each record
            //We use these extra condition for checking the string join
			if (r==1)
            {
                strcpy(record1.str,"Hola");
                strcpy(record2.str,"Hola");
            }
			record1.valid = true;
			record2.valid = true;
            // copy records to block
            memcpy(&block1.entries[r], &record1, sizeof(record_t));
			memcpy(&block2.entries[r], &record2, sizeof(record_t));
		}
		block1.nreserved = MAX_RECORDS_PER_BLOCK;
		block1.valid = true;
		block1.dummy = MAX_RECORDS_PER_BLOCK;
		fwrite(&block1, 1, sizeof(block_t), outfile); // write the block to the file

		block2.nreserved = MAX_RECORDS_PER_BLOCK;
		block2.valid = true;
		block2.dummy = MAX_RECORDS_PER_BLOCK;
		fwrite(&block2, 1, sizeof(block_t), outfile2); // write the block to the file
	}
	fclose(outfile);
	fclose(outfile2);

    block_t *buffer = NULL;
    unsigned int nios;

	//-----------------------------MERGE SORT-----------------------------------
    /*cout<<endl<<"--------------------MERGE SORT-------------------"<<endl<<endl;
	char filename[] = "file.bin";
	char *outputfile = new char[30]; //Big enough for a file name
	unsigned int segmentsNumber;
	unsigned int sortingPhases;
    MergeSort(filename,'1',buffer,50,outputfile,&segmentsNumber,&sortingPhases,
              &nios);
    cout<<"TOTAL PASSES: "<<sortingPhases<<endl;
    cout<<"SORTED SEGMENTS: "<<segmentsNumber<<endl;
    cout<<"NUMBER OF IOs: "<<nios<<endl;
    cout<<"OUTFILE: "<<outputfile<<endl;*/

    //-------------------------ELIMINATE DUPLICATES-----------------------------
    /*cout<<endl<<"--------------ELIMINATE DUPLICATES---------------"<<endl<<endl;
    char outputfileunique[] = "NOduplicates.bin";
    unsigned int uniquerecords;
    EliminateDuplicates (filename, '2', buffer,20, outputfileunique,&uniquerecords,
                         &nios);
    cout<<"UNIQUE RECORDS: "<<uniquerecords<<" OUT OF "
        <<nblocks*MAX_RECORDS_PER_BLOCK<<endl;
    cout<<"NUMBER OF IOs (including the merge sort IOs): "<<nios<<endl;
    cout<<"OUTFILE: "<<outputfileunique<<endl;*/

    //------------------------MERGE JOIN---------------------------
    cout<<endl<<"--------------MERGE JOIN-------------------"<<endl<<endl;
    unsigned int nres;
    char filename1[]= "file.bin";
    char filename2[]= "file2.bin";
    char outmerge[]= "outmerge.bin";
    MergeJoin(filename1,filename2,'1',buffer,100,outmerge,&nres,&nios);
    cout<<"PAIRS IN THE OUTPUT: "<<nres<<" OUT OF "<<nblocks*MAX_RECORDS_PER_BLOCK<<endl;
    cout<<"NUMBER OF IOs (including the eliminate duplicates IOs): "<<nios<<endl;

    //------------------------HASH JOIN---------------------------
    cout<<endl<<"--------------HASH JOIN-------------------"<<endl<<endl;
    char outhash[]="outhash.bin";
    HashJoin("1outfile.bin","2outfile.bin",'1',buffer,100,outhash,&nres,&nios);
    cout<<"PAIRS IN THE OUTPUT: "<<nres<<" OUT OF "<<nblocks*MAX_RECORDS_PER_BLOCK<<endl;
    cout<<"NUMBER OF IOs: "<<nios<<endl;

	/*
	 * If you want to remove the segments, you can do this:
	 * */
    #if defined(_WIN32) || defined(WIN32)
        system("del /Q segment*.bin");
    #else
        system("rm segment*.bin");
    #endif


	return 0;
}

