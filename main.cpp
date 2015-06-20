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
	int nblocks = 30;	// number of blocks in the file
	if (argc == 2) nblocks = atoi(argv[1]);

	record_t record1;
	record_t record2;

	block_t block1;
	block_t block2;

	unsigned int recid = 0;
	FILE *infile, *outfile, *outfile2;
	//----------------------GENERATION OF INITIAL FILES--------------------------
	cout<<"Creating input files..."<<endl;

	/* Sofia, whenever you're having problems for Windows and Linux, test for
	 * Windows using these macros! */

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
	outfile = fopen("file.bin", "wb");
	outfile2 = fopen("file2.bin", "wb");
#else
	outfile = fopen("file.bin", "w");
	outfile2 = fopen("file2.bin", "w");
#endif

	char* s = new char[10];

	for (int b=0; b<nblocks; ++b) { // for each block
		block1.blockid = b;
		block2.blockid = b;
		for (int r=0; r<MAX_RECORDS_PER_BLOCK; ++r) { // for each record
			// prepare a record for file 1
			record1.recid = recid;
			record1.num = rand() % (nblocks*20);
            gen_random_string(s,5);
			strcpy(record1.str,s);//Put a random string to each record

			// prepare a record for file 2
			record2.recid = recid++;
			record2.num = rand() % (nblocks*20);
            gen_random_string(s,5);
			strcpy(record2.str,s);//Put a random string to each record

			if (r==50)
            {
                strcpy(record1.str,"kalimera");
                strcpy(record2.str,"kalimera");
            }
            if  (r==60)
            {
                strcpy(record1.str,"hola");
                strcpy(record2.str,"hola");
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

	//-----------------------------MERGE SORT-----------------------------------
	cout<<endl<<"--------------------MERGE SORT-------------------"<<endl<<endl;
	block_t *buffer = NULL;
	char *outputfile = new char[30]; //Big enough for a file name
	unsigned int segmentsNumber;
	unsigned int sortingPhases;
	unsigned int IOsNumber;
	char filename[] = "file.bin";
    /*MergeSort(filename,'1',buffer,50,outputfile,&segmentsNumber,&sortingPhases,
              &IOsNumber);
    cout<<"TOTAL PASSES: "<<sortingPhases<<endl;
    cout<<"SORTED SEGMENTS: "<<segmentsNumber<<endl;
    cout<<"NUMBER OF IOs: "<<IOsNumber<<endl;
    cout<<"OUTFILE: "<<outputfile<<endl;*/

    //-------------------------ELIMINATE DUPLICATES-----------------------------
    /*cout<<endl<<"--------------ELIMINATE DUPLICATES---------------"<<endl<<endl;
    char outputfileunique[] = "NOduplicates.bin";
    unsigned int uniquerecords;
    EliminateDuplicates (filename, '2', buffer,20, outputfileunique,&uniquerecords,
                         &IOsNumber);
    cout<<"UNIQUE RECORDS: "<<uniquerecords<<" OUT OF "
        <<nblocks*MAX_RECORDS_PER_BLOCK<<endl;
    cout<<"NUMBER OF IOs (including the merge sort IOs): "<<IOsNumber<<endl;
    cout<<"OUTFILE: "<<outputfileunique<<endl;*/



    //------------------------MERGE JOIN---------------------------//
    cout<<endl<<"--------------MERGE JOIN-------------------"<<endl<<endl;
    char *mergeoutfile= new char[30];
    unsigned int nres;
    unsigned int nios;
    char filename1[]= "file.bin";
    char filename2[]= "file2.bin";
    char outmerge[]= "outmerge.bin";
    MergeJoin(filename1,filename2,'0',buffer,4,outmerge,&nres,&nios);
    cout<<"PAIRS IN THE OUTPUT: "<<nres<<" OUT OF "<<2*nblocks*MAX_RECORDS_PER_BLOCK<<endl;
    cout<<"NUMBER OF IOs (including the eliminate duplicates IOs): "<<nios<<endl;

    //------------------------HASH JOIN---------------------------//
    cout<<endl<<"--------------HASH JOIN-------------------"<<endl<<endl;
    char outhash[]="outhash.bin";
    HashJoin("1outfile.bin","2outfile.bin",'0',buffer,4,outhash,&nres,&nios);
    cout<<"PAIRS IN THE OUTPUT: "<<nres<<" OUT OF "<<2*nblocks*MAX_RECORDS_PER_BLOCK<<endl;
    cout<<"NUMBER OF IOs (including the eliminate duplicates IOs): "<<nios<<endl;
	// open file and print contents
	/*infile = fopen("file.bin", "r");
	int nreserved;
	while (!feof(infile)) { // while end-of-file has not been reached ...

		fread(&block, 1, sizeof(block_t), infile); // read the next block
		nreserved = block.nreserved;

		// print block contents
		for (int i=0; i<nreserved; ++i) {
			printf("this is block id: %d, record id: %d, num: %d, str: %s\n",
					block.blockid, block.entries[i].recid, block.entries[i].num, block.entries[i].str);
		}
	}

	fclose(infile);
	infile = fopen("file2.bin", "r");
	while (!feof(infile)) { // while end-of-file has not been reached ...

		fread(&block, 1, sizeof(block_t), infile); // read the next block
		nreserved = block.nreserved;

		// print block contents
		for (int i=0; i<nreserved; ++i) {
			printf("this is block id: %d, record id: %d, num: %d, str: %s\n",
					block.blockid, block.entries[i].recid, block.entries[i].num, block.entries[i].str);
		}
	}

	fclose(infile);*/

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

