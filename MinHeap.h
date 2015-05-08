#ifndef MINHEAP_H
#define MINHEAP_H

class MinHeap
{
    public:
        MinHeap(int heapSize);
        MinHeap();
        bool insertNumber(int number, int vertex);
        int removeRoot();
        bool isEmpty();
        int getRootVertex();
        void OutputArray();
        int* heapSort();
        int getHeapCurrentSize();
        int getMin();
        int getCost(int vertex,int &index);
        void replaceCost(int index, int newCost);
        bool searchVertex(int vertex);
    protected:
    private:
        int *heapArray;
        int *heapIndex; //Για τον αλγόριθμο Dijistra κρατάει και τον αριθμό του κόμβου που έχει ένα δεδομένο κόστος
        int heapMaxSize;
        int heapCurrentSize;
        int getRightChildIndex(int index);
        int getLeftChildIndex(int index);
        int getParentIndex(int index);
        void updateHeap(int childIndex);
        bool swapWithParent(int childIndex);
        int swapWithChild(int parentIndex);
        void Swap(int index1,int index2);
};

#endif // MINHEAP_H
