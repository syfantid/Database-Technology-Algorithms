#include "DatabaseProject.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <math.h>
#include <unordered_map>
#include <ciso646>
#include <algorithm>
#include <stdio.h>
#include <cassert>

using namespace std;

int num_open = 0;

struct CompareRecord : public std::binary_function<record_t*, record_t*, bool> {
	char field;
	CompareRecord(char field = 0): field(field) {}
	//CompareRecord(unsigned char field) { this->type = field; }
	bool operator()(const record_t* lhs, const record_t* rhs) const {
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
    int total = 0;
	//2 dummy variables just to call merge sort
	unsigned int segmentsNumber;
	unsigned int sortingPhases;

	unsigned int IOsNumber; //#of IOs performed including these of merge sort
	char *outofMerge= new char[30];
	MergeSort(infile,field,buffer,nmem_blocks,outofMerge,&segmentsNumber,&sortingPhases,
	          &IOsNumber);
	cout<<"Eliminating Duplicates..."<<endl;

	//Allocate memory to buffer; We need 2 blocks, 1 for input and 1 for output
	buffer = (block_t *) malloc (sizeof(block_t)*2);
	//Open file for reading
	FILE *inputfile, *outputfile;

    inputfile = fopen(outofMerge,FILE_READ); //Open the sorted file for reading
	outputfile = fopen(outfile,FILE_WRITE); //Open a file for output

	//Always keeping the previous record we read
	record_t previous;
	unsigned outindex = 0; //The index for the output buffer
	unsigned uniques = 0; //Number of unique records

	//cout<<"Eliminating Duplicates..."<<endl;
	while(!feof(inputfile)) { //while there are more blocks in the file
		//Read one sorted block at a time
		if(fread(&buffer[0], sizeof(block_t), 1, inputfile)!=1) {
            break;
		}
		//cout<<"New block"<<endl;
		for(unsigned i=0; i<buffer[0].nreserved; ++i) { //for all block entries
                total++;
            //cout<<"Record id: "<<buffer[0].entries[i].recid<<endl;
			bool same = false;
			if(field == '0') {
                if(compareID(&buffer[0].entries[i],&previous) == 0) {
                    same = true;
                }
			} else if(field == '1') {
			    if(compareNUM(&buffer[0].entries[i],&previous) == 0) {
                    same = true;
                }
			} else if(field == '2') {
			    if(compareSTR(&buffer[0].entries[i],&previous) == 0) {
                    same = true;
                }
			} else if(field == '3') {
			    if(compareNUMSTR(&buffer[0].entries[i],&previous) == 0) {
                    same = true;
                }
			}
			if(!same) { //It's not a duplicate
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
	//cout << "close: " << --num_open << endl;
	fclose(outputfile);
	//cout << "close: " << --num_open << endl;
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
		inputfile = fopen(infile,FILE_READ);
		//cout << "open: " << ++num_open << endl;
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
		//cout << "0." << endl;
		while(!feof(inputfile)) {
			//Read as many blocks as the buffer fits
			fread(buffer, nmem_blocks, sizeof(block_t), inputfile);
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
			outputfile = fopen(name.c_str(), FILE_WRITE);
			//cout << "open: " << ++num_open << endl;
			for (unsigned b=0; b<nmem_blocks; b++) {
				for (int i=0; i<MAX_RECORDS_PER_BLOCK && recSecIndex<recordsIndex; i++) {
					buffer[b].entries[i]=records[recSecIndex];
					recSecIndex++;
				}
				fwrite(&buffer[b],sizeof(block_t),1,outputfile);
				ios+=1;
			}
			fclose(outputfile);
			//cout << "close: " << --num_open << endl;

			//Emptying buffer
			memset(buffer, 0, nmem_blocks*sizeof(block_t));
			recordsIndex = 0;
		}
		//cout << "1." << endl;
		fclose(inputfile); //Closing the initial file
		//cout << "close: " << --num_open << endl;
		free(records);
		//N-WAY MERGE - STEP 1...N
		unsigned phase = 1; //Number of phases for the merging
		int filesInPhase = fileNumber;
		int filesProducedInPhase;
		int inputFileNumber = 1; //Current file number e.g.5
		int outputFileNumber = filesInPhase+1; //Next number of name available
		//e.g.segment6.bin
		int initialOutputFileNumber = outputFileNumber;
		do {
			//cout << "2." << endl;
			//While we have more than 1 file for the next round to merge -
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
				//cout << "3." << endl;
				int filesInRound = 0;
				/*cout<<"Files read in phase so far: "<<filesRead<<endl<<
				"Total files in phase: "<<filesInPhase<<endl;*/
				//While there are more current
				//files to be read in this round - WHILE FOR FILES IN ROUND
				string outputName = createFileName(outputFileNumber);
				//cout << "3.x going to open" << endl;
				outputfile = fopen(outputName.c_str(), FILE_WRITE);
				//cout << "open: " << ++num_open << endl;
                int openFiles = 0;
				for(unsigned b=0; b<nmem_blocks-1; ++b) { //For each n-1 files read

					//their first block into n-1 buffer blocks
					if(inputFileNumber<initialOutputFileNumber) {
						names[b] = createFileName(inputFileNumber);
						//cout << "3. going to open" << endl;
                        currentFiles[b] = fopen(names[b].c_str(),FILE_READ);
                        ++openFiles;
						//cout << "open: " << ++num_open << endl;

						if (currentFiles[b] == NULL) {
							cout << "fopen() file is NULL!" << endl;
							exit(-1);
						}

						//cout << "going to fread" << endl;
						fread(&buffer[b],sizeof(block_t),1,currentFiles[b]);
						//cout << "fread" << endl;


						//cout << "going to test feof" << endl;
						if(!feof(currentFiles[b])) {
							//cout << "!feof" << endl;
							++filesRead; //Each buffer block "reads" a different file
							++inputFileNumber; //Update the file number to be read next
							//cout << "buffer[b].entries[index[b]].dummy1 = b;" << endl;
							buffer[b].entries[index[b]].dummy1 = b;
							//Put a record inside the minheap
							//cout << "pq. push" << endl;
							pq.push(&buffer[b].entries[index[b]]);
							--buffer[b].dummy;
							//cout << " endif" << endl;
							++filesInRound;
						}
						//cout << "endif2" << endl;
					}
				}
				//cout << "3.1" << endl;
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
					//printRecord(*(record_t*)pq.top());
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
				//cout << "4." << endl;
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
					//printRecord(*(record_t*)pq.top());
					pq.pop();
				}
				//cout << "5." << endl;
				buffer[nmem_blocks-1].dummy = buffer[nmem_blocks-1].nreserved;
				fwrite(&buffer[nmem_blocks-1],sizeof(block_t),1,outputfile);
				ios+=1;
				memset(&buffer[nmem_blocks-1],0,sizeof(block_t));
				index[nmem_blocks-1] = 0;
				++outputFileNumber;
				++filesProducedInPhase;
				fclose(outputfile);
				//cout << "close: " << --num_open << endl;
				for(int i=0; i<openFiles; ++i) { //Closing current files
					fclose(currentFiles[i]); //disabled for Linux
					//cout << "close: " << --num_open << endl;
					remove(names[i].c_str()); //Deleting current files, so that
					//the disc isn't full of not needed files
				}
				//cout << "6." << endl;
				memset(buffer,0,nmem_blocks*sizeof(block_t));
				for(unsigned b=0; b<nmem_blocks; ++b) {
					index[b] = 0; //Each block's index returns to 0
				}
			}
			//cout << "7." << endl;
			phase+=1;
			fileNumber += filesProducedInPhase;
			outputFileNumber = inputFileNumber+filesProducedInPhase;
			initialOutputFileNumber = outputFileNumber;
			filesInPhase = filesProducedInPhase;

			//for (auto f : currentFiles) fclose(f);fc

		} while(filesProducedInPhase!=1);
		//cout << "4.";
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


void MergeJoin (char *infile1, char *infile2, unsigned char field, block_t *buffer, unsigned int nmem_blocks, char *outfile, unsigned int *nres, unsigned int *nios) {
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

    inputFile_R= fopen(outfile_R,FILE_READ);
	assert(inputFile_R);
	//cout << "open: " << ++num_open << endl;
	//cout<<outfile_R<<endl;
    inputFile_S= fopen(outfile_S,FILE_READ);
	assert(inputFile_S);
	//cout << "open: " << ++num_open << endl;
    outputFile=fopen(outfile,FILE_WRITE);
	//cout << "open: " << ++num_open << endl;
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
	while(!feof(inputFile_R)) { //end of input files

		if(bufOutIndex<nmem_blocks) { //end of out buffers

			if(indexR<=buffer[0].nreserved-1) { //index of first file smaller than entries
				if (indexS<=buffer[1].nreserved-1) { //index of second file
					if(field=='0') { //join with ID

						result= compareID(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);

					} else if(field=='1') { //join with num
						result= compareNUM(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);

					} else if(field=='2') { //join with STR
						result= compareSTR(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);
					} else if(field=='3') { //join with num and STR
						result= compareNUMSTR(&buffer[0].entries[indexR],&buffer[1].entries[indexS]);
					}

					//cout<<result<<endl;
					if (bufOutEntrIndex== MAX_RECORDS_PER_BLOCK) { //if out buffer is full
						//cout<<"5"<<endl;
						if(bufOutIndex<nmem_blocks-1) { //if there is available outbuffer index
							//cout<<"1"<<endl;
							bufOutIndex++;
							bufOutEntrIndex=0;

						} else { //if not
							for(unsigned int k=2; k<nmem_blocks; k++) { //write the buffers to file
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
					} else {
						if (result==-1) { //R file's record is smaller than S file's record (sorted with mergesort);
							indexR++;//next record of R file
							//cout<<"-1"<<endl;
						} else if(result==1) { //S file's record is smaller than R file's record
							//cout<<"1"<<endl;
							indexS++;//next record of S file
						} else {

							memcpy(&buffer[bufOutIndex].entries[bufOutEntrIndex],&buffer[0].entries[indexR],sizeof(record_t));//join random (here with the R record)
							//printRecord(buffer[bufOutIndex].entries[bufOutEntrIndex]);
							//cout<<"l"<<endl;
							indexR++;
							indexS++;
							bufOutEntrIndex++;
							counter++;


						}
					}
				} else {
					memset(&buffer[1],0,sizeof(block_t));
					fread(&buffer[1],sizeof(block_t),1,inputFile_S);
					numberofIOS++; //reading
					if (buffer[1].nreserved==0) {
						break;
					}
					indexS=0;
				}
			} else {
				memset(&buffer[0],0,sizeof(block_t));
				fread(&buffer[0],sizeof(block_t),1,inputFile_R);
				numberofIOS++; //reading
				if (buffer[0].nreserved==0) {
					break;
				}
				indexR=0;
			}
		}
	}
	for(unsigned int k=2; k<nmem_blocks; k++) { //write the remaining buffers to file
		if(buffer[k].nreserved!=0) {
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
	//cout << "close: " << --num_open << endl;
	fclose(inputFile_S);
	//cout << "close: " << --num_open << endl;
	fclose(outputFile);
	//cout << "close: " << --num_open << endl;
	*nres=counter;
	*nios=numberofIOS;

}

void HashJoin (char *infile1, char *infile2, unsigned char field, block_t *buffer, unsigned int nmem_blocks, char *outfile, unsigned int *nres, unsigned int *nios) {

	FILE *in1, *in2, *out;
    in1= fopen(infile1,FILE_READ);
	//cout << "open: " << ++num_open << endl;
	buffer = (block_t *) malloc (sizeof(block_t)*nmem_blocks);
	unordered_map<unsigned int,int> m1;
	unordered_map<string,int> m2;//string cause its easier to handle- int dummy
	unordered_multimap<unsigned int,string> m3;//in case of field 1 or 2 , vector is dummy
	vector<string> insVect;//vector to insert to the map
    int numberofIOS=0;

    //Here starts the Hashtable building phase

	while(!feof(in1)) { //While the file is not finished
        memset(buffer, 0, nmem_blocks*sizeof(block_t));
		//Read as many blocks as the buffer fits
		fread(buffer, sizeof(block_t), nmem_blocks-1, in1);
        numberofIOS++;
		if(buffer[0].nreserved == 0) { //Because FEOF is set AFTER the end of file
			break;
		}
		//For each block
		for (unsigned b=0; b<nmem_blocks; b++) {
			int nreserved = buffer[b].nreserved;
			//For each record in the block
			for (int i=0; i<nreserved; ++i) {
				if(field == '0')
					m1.insert(make_pair(buffer[b].entries[i].recid,0));//insert the id
				else if(field == '1')
					m1.insert(make_pair(buffer[b].entries[i].num,0));//insert the num
				else if(field == '2')
				{
                    string str(buffer[b].entries[i].str);//we create a string of the str var and insert it to hashmap
					m2.insert(make_pair(str,0));
                }
				else if(field == '3') {
                    string str(buffer[b].entries[i].str);
                    m3.insert(make_pair(buffer[b].entries[i].num,str));
					//unordered_map<unsigned int, vector<string>>::const_iterator rec=m3.find(buffer[b].entries[i].num);
					//if(rec != m3.end()) { //found
						/* I added a const_cast because you can't modify
						* a const_iterator. Hack :P, but it doesn't compile
						* otherwise. */
						//vector<string>& v = const_cast<vector<string>&>(rec->second);

						/*
						 * Possible bug?
						 * std::find uses operator== to search for elements that
						 * compare the same. However, if you compare two char
						 * pointers (char*) with ==, it won't say whether the
						 * strings are the same, it will rather say whether the
						 * POINTERS are the same (most likely not). My
						 * suggestion is to use std::string, or if you want to
						 * keep using char*, std::find_if with a lambda.
						 * */

						/* Also here you were using just
						 * &buffer[b].entries[i].str, so you were comparing a
						 * char* with a char**, which g++ might allow you to do
						 * but it's not right :) fixed it.*/
						/*if (find(v.begin(), v.end(), &buffer[b].entries[i].str[0])==v.end()) {
							v.push_back(&buffer[b].entries[i].str[0]);
						}*/
						/*string hString(buffer[b].entries[i].str);
						if (find(v.begin(),v.end(),hString)==v.end())
						{
                            v.push_back(hString);
						}

					}
					else{//not found
                        //cout<<"he"<<endl;
                        string hString(buffer[b].entries[i].str);//string to handle vector better
                        insVect.push_back(hString);//insert string to vector
                        m3.insert(make_pair(buffer[b].entries[i].num,insVect));
					}*/
				}
			}
		}
	}
	fclose(in1);

	//Here starts the probing phase

    in2=fopen(infile2,FILE_READ);
    out=fopen(outfile,FILE_WRITE);
	int outBufferIndex=0;
	int blockID=0;

	int counter=0;
	block_t * buffer2 = (block_t *) malloc (sizeof(block_t)*nmem_blocks);//second buffer because hashmap saves the pointer and not the value
	while(!feof(in2))
	{
        fread(buffer2, sizeof(block_t), nmem_blocks-1,in2);//reading nmem_blocks-1 buffers, we leave one for the output
        numberofIOS++;
		if(buffer2[0].nreserved == 0) { //Because FEOF is set AFTER the end of file
			break;
		}
		for (unsigned b=0; b<nmem_blocks-1; b++) {
			int nreserved = buffer2[b].nreserved;
			for (int i=0; i<nreserved; ++i) {
                if (outBufferIndex == MAX_RECORDS_PER_BLOCK)//if out buffer is full
                {
                    buffer2[nmem_blocks-1].blockid=blockID;//insert the block details
                    blockID++;
                    buffer2[nmem_blocks-1].valid=true;
                    buffer2[nmem_blocks-1].nreserved= MAX_RECORDS_PER_BLOCK;
                    fwrite(&buffer2[nmem_blocks-1],sizeof(block_t),1,out);//write
                    numberofIOS++;//each writing
                    memset(&buffer[nmem_blocks-1],0,sizeof(block_t));//flush the buffer
                    outBufferIndex=0;
                }
                if(field == '0')//recID
                {
                    unordered_map<unsigned int, int>::const_iterator rec=m1.find(buffer2[b].entries[i].recid);//iterator to find the value
                    if (rec!=m1.end())//found
                    {
                        memcpy(&buffer2[nmem_blocks-1].entries[outBufferIndex],&buffer2[b].entries[i],sizeof(record_t));//join with the seconds file's record
                        outBufferIndex++;
                        counter++;
                    }
                }
                else if(field == '1')//num
                {
                    unordered_map<unsigned int, int>::const_iterator rec=m1.find(buffer2[b].entries[i].num);//iterator to find the value
                    if (rec!=m1.end())//found
                    {
                        memcpy(&buffer2[nmem_blocks-1].entries[outBufferIndex],&buffer2[b].entries[i],sizeof(record_t));//join with the second file's record
                        outBufferIndex++;
                        counter++;
                    }
                }
                else if(field == '2')//str
                {
                    string str(buffer2[b].entries[i].str);//"converting" to string- easier handling

                    unordered_map<string, int>::const_iterator rec=m2.find(str);//iterator to find the value
                    if (rec!=m2.end())//found
                    {
                        memcpy(&buffer2[nmem_blocks-1].entries[outBufferIndex],&buffer2[b].entries[i],sizeof(record_t));//join with the second file's record
                        outBufferIndex++;
                        counter++;
                    }
                }
                else if(field == '3')//num and str
                {
                    string hString(buffer2[b].entries[i].str);
                    auto range= m3.equal_range(buffer2[b].entries[i].num);
                    for (auto it = range.first;it !=range.second;++it)
                    {
                           if(it->second== hString)
                           {
                            memcpy(&buffer2[nmem_blocks-1].entries[outBufferIndex],&buffer2[b].entries[i],sizeof(record_t));//join with the second file's record
                            outBufferIndex++;
                            counter++;
                        }
                    }
                }
            }
        }
	}
	buffer2[nmem_blocks-1].blockid=blockID;//insert the block details
    buffer2[nmem_blocks-1].valid=true;
    buffer2[nmem_blocks-1].nreserved= counter;
    fwrite(&buffer2[nmem_blocks-1],sizeof(block_t),1,out);//write
    numberofIOS++;//each writing
    memset(&buffer[nmem_blocks-1],0,sizeof(block_t));//flush the buffer
    free(buffer);
    free(buffer2);
    fclose(in2);
    fclose(out);
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



