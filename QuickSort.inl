#include "QuickSort.h"
#include <algorithm>

template<class T>
QuickSort<T>::QuickSort(T *a,int n)
{
    /*QuickSort(a, 0, n-1)
    What do you mean by this? This function does not exist...
    */
}

template<class T>
void quickSort(T a[],int l,int r)
{
    std::sort(a + l, a + r);
    return;
  /*  if (l>= r) return ;
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
        }while(a[j]>pivot);
        if (i>=j) break;
        //Swap
    }
*/
}
