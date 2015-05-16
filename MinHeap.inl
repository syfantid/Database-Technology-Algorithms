#include "MinHeap.h"

template<class T>
MinHeap<T>::MinHeap(int MinHeapSize)
{
    MaxSize= MinHeapSize;
    heap = new T [MaxSize+1];
    CurrentSize=0;
}

template<class T>
MinHeap<T>& MinHeap<T>::Insert(const T& x)
{
    if(CurrentSize== MaxSize)
    {
        cout<<"out of range"<<endl;
    }
    int i= ++CurrentSize;
    while(i != 1 && x< heap[i/2])
    {
        heap[i] = heap[i/2];
        i/=2;
    }

    heap[i]=x;
    return *this;
}

template<class T>
MinHeap<T>& MinHeap<T>::DeleteMin(T& x)
{
    if (CurrentSize == 0)
        cout<<"Minheap empty";
    x=heap[1];
    T y=heap[CurrentSize--];

    int i=1;
    int ci=2;
    while (ci<=CurrentSize)
    {
        if(ci < CurrentSize && heap[ci] > heap[ci+1])
        {
            ci++;
        }
        if (y <=heap[ci]) break;

        heap[i]= heap[ci];
        i=ci;
        ci *=2;

    }
    heap[i]=y;
    return *this;
}

