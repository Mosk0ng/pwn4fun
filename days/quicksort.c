#include <stdio.h>
#include <stdlib.h>

void show(int *, int);
void swap(int *, int , int );
void quicksort(int *, int ,int );

void swap(int * array, int i, int j)
{
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

void quicksort(int * array, int begin, int end)
{
    if(begin >= end){
        return;
    }
    int i = begin;
    int j = end;
    while(i!=j){
        while(j!=i){
            if(array[j]>=array[begin]) j--;
            else break;
        }
        while(i!=j){
            if(array[i]<=array[begin]) i++;
            else break;
        }
        swap(array,i,j);
    }
    swap(array,begin,i);
    //show(array,end-begin+1);
    quicksort(array,begin,i-1);
    quicksort(array,i+1,end);
}

void show(int * array,int size){
    for(int i = 0; i < size; i++){
        printf("%d ", array[i]);
    }
    puts("\n");
}

int main()
{
    int array[] = {6,1,2,7,9,3,4,5,10,8};
    quicksort(array, 0, 9);
    show(array,10);
}