#include "DatabaseProject.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {

	int nblocks = 20;	// number of blocks in the file
	record_t record;
	block_t block;
	unsigned int recid = 0;
	FILE *infile, *outfile;

	//Generate a file
	outfile = fopen("file.bin", "w");

	for (int b=0; b<nblocks; ++b) { // for each block

		block.blockid = b;
		for (int r=0; r<MAX_RECORDS_PER_BLOCK; ++r) { // for each record

			// prepare a record
			record.recid = recid++;
			record.num = rand() % 1000;
			strcpy(record.str,"hello");   // put the same string to all records
			record.valid = true;

			memcpy(&block.entries[r], &record, sizeof(record_t)); // copy record to block
		}

		block.nreserved = MAX_RECORDS_PER_BLOCK;
		block.valid = true;
		block.dummy = MAX_RECORDS_PER_BLOCK;
		fwrite(&block, 1, sizeof(block_t), outfile);	// write the block to the file
	}
    //cout<<"lice";
	fclose(outfile);
	block_t *buffer = NULL;
	char *outputfile = NULL;
	unsigned int segmentsNumber;
	unsigned int sortingPhases;
	unsigned int IOsNumber;
	char filename[] = "file.bin";
    MergeSort(filename,'1',buffer,4,outputfile,&segmentsNumber,&sortingPhases,&IOsNumber);
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

