

#include "HashTable.h"
#include <iostream>


using namespace std;

 //class Element, class Key
template <class E, class K> HashTable<E,K>::HashTable(int divisor)
{
    D=divisor;

    //allocate memory for the arrays
    ht=new E[D];
    empt= new bool[D];

    //set alla buckets to true (that is empty)
    for (int i=0;i<D;i++)
    {
        empt[i]=true;

    }
}
template <class E, class K> //class Element, class Key
HashTable<E,K>::~HashTable()
{
    delete [] ht;
    delete [] empt;
}
template <class E,class K>
int HashTable<E,K>::hSearch(const K& k) const
{

    //Search the table(open adressed)
    //return location of key(k)
    //Otherwise return where the element is
    int i=k%D; //first bucket
    int j=i; //start at first bucket
    do
    {
        if (empt[j] || ht[j]==k)
        {
            return j;
        }
        j=(j+1)%D; //next bucket
    }while (j!=i); //if is the first bucket

    return j; //full table
}
template <class E,class K>
bool HashTable<E,K>::Search(const K& k,int &index) const
{
    //Put the element that is "connected" to the Key k.
    //return true if there is or false if not
    int b=hSearch(k);
    if (empt[b] || ht[b] !=k)
    {
        return false;//if no match
    }
    index=b;
    return true;
}
template <class E,class K>
bool HashTable<E,K>::Insert(const E& e,int &index)
{
    //Insert to hash
    K k= e;
    int b=hSearch(k);//find key

    if (empt[b])//never never used
    {
        empt[b]=false;
        ht[b]=e;
        index=b;
        return true;
    }
    //no insert, check if double or full
    if (ht[b]==k)
    {
        index=b;
        return false;
       //cout<<"\n Duplicate"<<endl;
    }
    else
    {
        //cout<<"\n No memory"<<endl;
        return false;
    }

}

template <class E,class K>
void HashTable<E,K>::hDelete(E e)
{
    K b=hSearch(e);
    empt[b]=true;
    ht[b]=-1;
}
template <class E,class K>
bool HashTable<E,K>::getEmpt(K k)
{
    return empt[k];
}
template <class E,class K>
E HashTable<E,K>::getHT(K k)
{
    return ht[k];
}

