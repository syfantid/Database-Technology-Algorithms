#include "QuickSort.h"
template<class T>
QuickSort::QuickSort(T *a,int n)
{
    QuickSort(a,0,n-1)
}

template<class T>
void quickSort(T a[],int l,int r)
{
    if (l>= r) return ;
    int i=l,
        j=r+1;
    T pivot = a[1];
    while (true)
    {
        do
        {
            i=i+1;
        }while (a[i]<pivot);
        do
        {
            j=j-1;
        }while(a[j]>pivot)
        if (i>=j) break;
        Swap
    }

}
