#ifndef HASHTABLE_H
#define HASHTABLE_H

template <class E,class K>
class HashTable
{
    public:

        HashTable(int divisor);
        ~HashTable();
        bool Search(const K& k,int &index) const;
        bool Insert(const E& e,int &index);
        int hSearch(const K& k) const;
        void hDelete(E e);
        bool getEmpt(K k);
        E getHT(K k);
    private:
        //template <class E,class K>;

        int D; //divisor
        E *ht; //hash table Array
        bool *empt; //buckets
};

#endif // HASHTABLE_H
