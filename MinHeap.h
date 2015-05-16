#ifndef MINHEAP_H
#define MINHEAP_H
#include <iostream>
using namespace std;
template<class T>
class MinHeap
{
    public:
        MinHeap(int MinHeapSize);
        int Size() const {return CurrentSize;}
        T Min() {if (CurrentSize == 0)
                    cout<<"Minheap Problem"<<endl;
                return heap[1];}
        MinHeap<T>& Insert(const T& x);
        MinHeap<T>& DeleteMin(T& x);
        //void Initialize(T a[], int size, int ArraySize;)
    protected:
    private:
        int CurrentSize, MaxSize;
        T *heap;
};

#include "MinHeap.inl"

#endif // MINHEAP_H
