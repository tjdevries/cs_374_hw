/* 
 *Ryan Siekman
 *CS374 
 *Reduction function for pthread
 *Project 6
 *
 * Function needs ThreadID and numThreads from main
 */

#include <pthread.h>
#include "pthreadBarrier.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

long double* treeArray;
int treeLevel;
double maxLevel;
unsigned long Threads = 0;
int num = 0;
pthread_mutex_t piLock;


void pthreadReductionSum(long double lSum, void * arg){
    //Need to sum the lSum in a tree structure
    //so make temp variables to fill in the tree
    //if 8 to reduce, 4 needed, if 9, still 4
    //so floor(numProc)
    //so if x number of processes to sum then floor(x^.5) for the number of levels to the tree    
    //int ID = pthread_self();
    pthread_mutex_lock(&piLock);
    int ID = Threads;
    Threads ++;
    pthread_mutex_unlock(&piLock);
    printf("Thread %i here\n", ID);
    pthreadBarrier(Threads);
    if( ID == 0){
        printf("Number of Threads %lu\n", Threads);
        treeArray = (long double*) malloc( Threads * sizeof(long double));
        treeLevel = 1;
        maxLevel = floor(log2(Threads));
    }
    pthreadBarrier(Threads); //so that array exists before we write to it
    treeArray[ID] = lSum; //tree array starts values at 1
    pthreadBarrier(Threads); //wait here for all values to be entered at base of tree
    if((ID % 2) == 1){ pthread_exit(NULL); }
    while( treeLevel <= maxLevel ){
        printf("Thread %i here, treelevel %i\n", ID, treeLevel);
        if(ID % (int)(pow( 2, treeLevel )) == 0){
            printf("In if Thread %i here, treelevel %i\n", ID, treeLevel);
            treeArray[ID] = treeArray[ID] + treeArray[ID + (int)(pow( 2, treeLevel - 1 ))];
        }
        else{ pthread_exit(NULL); }//exit threads that have finished
        if( ID == 0 ){ treeLevel ++; }  //this increases the depth of the tree as more threads are reduced
        pthreadBarrier(Threads / pow( 2, treeLevel ));
    }
    if( (Threads % 2) == 1 ){ //if odd number of threads add the outlier 
        treeArray[0] = treeArray[0] + treeArray[Threads]; }
    if( ID == 0 ){ 
        arg = &treeArray[0]; }
}

