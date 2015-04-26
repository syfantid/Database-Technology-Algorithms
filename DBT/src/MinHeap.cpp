#include "MinHeap.h"
#include <cstdlib>
#include <iostream>

using namespace std;

MinHeap::MinHeap(int heapSize)
{
    heapArray = new(nothrow) int[heapSize];
    heapMaxSize = heapSize;
    heapCurrentSize = 0;
    heapIndex = new(nothrow) int[heapSize];
}

MinHeap::MinHeap()
{
    heapArray = NULL;
    heapIndex = NULL;
    heapMaxSize = heapCurrentSize = 0;
}

int MinHeap::getHeapCurrentSize()
{
    return heapCurrentSize;
}

int MinHeap::getRightChildIndex(int index)
{
    return 2*index+2;
}

int MinHeap::getLeftChildIndex(int index)
{
    return 2*index+1;
}

int MinHeap::getParentIndex(int index)
{
    if(index == 0)
    {
        return -1;
    }
    return (index-1)/2;
}

bool MinHeap::swapWithParent(int childIndex)
{
    if(childIndex < 1) //It means we're at the first level of the heap
    {
        return false;
    }
    int parent = getParentIndex(childIndex);
    Swap(parent,childIndex);
    return true;
}

void MinHeap::Swap(int index1,int index2)
{
    int temp = heapArray[index1];
    heapArray[index1] = heapArray[index2];
    heapArray[index2] = temp;

    temp = heapIndex[index1];
    heapIndex[index1] = heapIndex[index2];
    heapIndex[index2] = temp;
}

int MinHeap::swapWithChild(int parentIndex)
{
    int leftChildIndex = getLeftChildIndex(parentIndex);
    int rightChildIndex = getRightChildIndex(parentIndex);
    if(leftChildIndex >= heapCurrentSize) //There are no more children (leaves)
    {
        return -1;
    }
    if(heapArray[rightChildIndex] < heapArray[leftChildIndex])
    {
        Swap(parentIndex,rightChildIndex);
        return rightChildIndex;
    }
    else
    {
        Swap(parentIndex,leftChildIndex);
        return leftChildIndex;
    }
}

bool MinHeap::insertNumber(int number, int vertex)
{
    if(heapCurrentSize == heapMaxSize) //Heap is full
    {
        return false;
    }
    heapArray[heapCurrentSize] = number;
    heapIndex[heapCurrentSize] = vertex;
    int childIndex = heapCurrentSize;
    ++heapCurrentSize;
    updateHeap(childIndex);
    return true;
}

int MinHeap::getMin()
{
    return heapArray[0];
}

int MinHeap::removeRoot()
{
    if(heapCurrentSize==0) //Empty heap
    {
        return 1000000;
    }
    int temp = heapArray[0];
    heapArray[0] = heapArray[heapCurrentSize-1]; //The last inserted element becomes root
    heapIndex[0] = heapIndex[heapCurrentSize-1];
    --heapCurrentSize;
    if(heapCurrentSize != 0) { //Only root was left before deletion
        int leftChildIndex = getLeftChildIndex(0);
        int rightChildIndex = getRightChildIndex(0);
        int parentIndex = 0;
        while(heapArray[parentIndex]>heapArray[leftChildIndex] || heapArray[parentIndex]>heapArray[rightChildIndex])
        {
            parentIndex = swapWithChild(parentIndex);
            if(parentIndex == -1)
            {
                break;
            }
            leftChildIndex = getLeftChildIndex(parentIndex);
            rightChildIndex = getRightChildIndex(parentIndex);
        }
    }
    return temp;
}

int* MinHeap::heapSort()
{
    int tempSize = heapCurrentSize;
    int* temp = new int[heapCurrentSize]; //We'll keep the initial array in a temporary variable
    for(int i=0; i<heapCurrentSize; i++)
    {
        temp[i] = heapArray[i];
    }
    int* sortedArray = new int[heapCurrentSize];
    for(int i=0; i<tempSize; i++)
    {
        sortedArray[i] = removeRoot(); //Root is always the smallest element of all
    }
    heapArray = temp;
    heapCurrentSize = tempSize;
    return sortedArray;
}

int MinHeap::getRootVertex() {
    if(heapCurrentSize != 0)
    {
        return heapIndex[0];
    }
    return -1;
}

bool MinHeap::isEmpty() {
    return (heapCurrentSize==0);
}

void MinHeap::OutputArray() {
	for (int i = 0; i < heapCurrentSize; ++i) {
		cout << heapArray[i] << "  ";
	}
	cout << endl;
}

//Επιστρέφει τον αριθμό-κόστος που αντιστοιχεί στον κόμβο-παράμετρο
int MinHeap::getCost(int vertex,int &index) {
    bool found = false;
    int counter = 0;
    while(!found)
    {
        if(heapIndex[counter] == vertex)
        {
            index = counter;
            return heapArray[counter];
        }
        ++counter;
    }
    return -1;
}

void MinHeap::replaceCost(int index, int newCost)
{
    //cout<<endl<<"replaceCost function"<<endl<<"Index: "<<index<<" newcost: "<<newCost<<" oldCost: "<<heapArray[index]<<endl;
    heapArray[index] = newCost;
    updateHeap(index);
}

void MinHeap::updateHeap(int childIndex)
{
    //cout<<endl<<"updateHeap function called!"<<endl;
    int parentIndex = getParentIndex(childIndex);
    /*cout<<endl<<"Parent index: "<<parentIndex<<" Child index: "<<childIndex<<endl;
    //cout<<"Before Updating: "<<endl;
    OutputArray();*/
    while(parentIndex >= 0)
    {
        if(heapArray[parentIndex] > heapArray[childIndex])
        {
            swapWithParent(childIndex);
            childIndex = parentIndex;
            parentIndex = getParentIndex(parentIndex);
        }
        else //Number is at the right place
        {
            break;
        }
    }
    //cout<<"After Updating: "<<endl;
    //OutputArray();
}

bool MinHeap::searchVertex(int vertex) {
    bool found = false;
    int counter = 0;
    while(!found && counter<heapCurrentSize)
    {
        if(heapIndex[counter] == vertex)
        {
            return true;
        }
        ++counter;
    }
    return false;
}
