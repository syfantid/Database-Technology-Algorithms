#include "DatabaseProject.h"
#include <iostream>
#include <ctime>

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
	int nblocks = 200;	// number of blocks in the file
	record_t record;
	block_t block;
	unsigned int recid = 0;
	FILE *infile, *outfile;
	//----------------------GENERATION OF INITIAL FILE--------------------------
	cout<<"Creating input file..."<<endl;
	outfile = fopen("file.bin", "wb");
    char* s = new char[10];

	for (int b=0; b<nblocks; ++b) { // for each block
		block.blockid = b;
		for (int r=0; r<MAX_RECORDS_PER_BLOCK; ++r) { // for each record
			// prepare a record
			record.recid = recid++;
			record.num = rand() % (nblocks*20);
            gen_random_string(s,10);
			strcpy(record.str,s);   //Put a random string to each record
			record.valid = true;

			memcpy(&block.entries[r], &record, sizeof(record_t)); // copy record to block
		}
		block.nreserved = MAX_RECORDS_PER_BLOCK;
		block.valid = true;
		block.dummy = MAX_RECORDS_PER_BLOCK;
		fwrite(&block, 1, sizeof(block_t), outfile);	// write the block to the file
	}
	fclose(outfile);

	//-----------------------------MERGE SORT-----------------------------------
	cout<<endl<<"--------------------MERGE SORT-------------------"<<endl<<endl;
	block_t *buffer = NULL;
	char *outputfile = new char[30]; //Big enough for a file name
	unsigned int segmentsNumber;
	unsigned int sortingPhases;
	unsigned int IOsNumber;
	char filename[] = "file.bin";
    MergeSort(filename,'1',buffer,4,outputfile,&segmentsNumber,&sortingPhases,
              &IOsNumber);
    cout<<"TOTAL PASSES: "<<sortingPhases<<endl;
    cout<<"SORTED SEGMENTS: "<<segmentsNumber<<endl;
    cout<<"NUMBER OF IOs: "<<IOsNumber<<endl;
    cout<<"OUTFILE: "<<outputfile<<endl;

    //-------------------------ELIMINATE DUPLICATES-----------------------------
    cout<<endl<<"--------------ELIMINATE DUPLICATES---------------"<<endl<<endl;
    char *outputfileunique = new char[30]; //Big enough for a file name
    unsigned int uniquerecords;
    EliminateDuplicates (filename, '1', buffer,4, outputfileunique,&uniquerecords,
                         &IOsNumber);
    cout<<"UNIQUE RECORDS: "<<uniquerecords<<" OUT OF "
        <<nblocks*MAX_RECORDS_PER_BLOCK<<endl;
    cout<<"NUMBER OF IOs (including the merge sort IOs): "<<IOsNumber<<endl;
    cout<<"OUTFILE: "<<outputfileunique<<endl;


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

	fclose(infile);*/
	return 0;
}

