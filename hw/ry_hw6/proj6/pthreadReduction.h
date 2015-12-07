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
pthread_mutex_t piLock;


void pthreadReductionSum(long double lSum, void * arg, unsigned long Threads, int ID){
    //Need to sum the lSum in a tree structure
    //so make temp variables to fill in the tree
    //if 8 to reduce, 4 needed, if 9, still 4
    //so floor(numProc)
    //so if x number of processes to sum then floor(x^.5) for the number of levels to the tree    
    //int ID = pthread_self();
    //printf("Thread %i here, value: %Lf\n", ID, lSum);
    if( ID == 0){
        //printf("Number of Threads %lu\n", Threads);
        treeArray = (long double*) malloc( Threads * sizeof(long double));
        treeLevel = 1;
        maxLevel = floor(log2(Threads));
    }
    pthreadBarrier(Threads); //so that array exists before we write to it
    treeArray[ID] = lSum; //tree array starts values at 1
    pthreadBarrier(Threads); //wait here for all values to be entered at base of tree
    if((ID % 2) == 1){ pthread_exit(NULL); } //kill every other thread
    while( treeLevel <= maxLevel ){
        //printf("Thread %i here, treelevel %i, array val: %Lf\n", ID, treeLevel, treeArray[ID]);
        if(ID % (int)(pow( 2, treeLevel )) == 0){
            //printf("In if Thread %i here, treelevel %i\n", ID, treeLevel);
            treeArray[ID] = treeArray[ID] + treeArray[ID + (int)(pow( 2, treeLevel - 1 ))];
        }
        else{ pthread_exit(NULL); }//exit threads that have finished
        pthreadBarrier(Threads / pow( 2, treeLevel ));
        if( ID == 0 ){ treeLevel ++; }  //this increases the depth of the tree as more threads are reduced
        pthreadBarrier(Threads / pow( 2, treeLevel-1 ));
    }
    if( (Threads % 2) == 1 ){ //if odd number of threads add the outlier 
        treeArray[0] = treeArray[0] + treeArray[Threads - 1]; 
        pthread_exit(NULL);
    }
    if( ID == 0 ){
        //printf("at end in thread 0, results: %Lf\n", treeArray[0]);
        //int * newArg = (int *) arg;
        //*newArg = treeArray[0];
        memcpy(arg, &treeArray[0], 1*sizeof(long double));
        pthread_exit(NULL);
    }
}

