/* mpiArraySum.c is a function that takes in an array and returns the sum
 *  It has parallel functionality added by MPI
 * TJ DeVries, Fall 2015 
 * for CS 374 (HPC) at Calvin College.
 */

#include <stdio.h>      /* I/O stuff */
#include <stdlib.h>     /* calloc, etc. */
#include <mpi/mpi.h>    /* MPI functinality */

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;
double parallelSumArray(double *a, int howMany, int id, int numProc);

int debug = 0;

int main(int argc, char * argv[])
{
    int  howMany;
    double localSum, finalSum;
    double * masterArray;

    double t1, t2;

    /* MPI Initialization */
    int numProc = -1;
    int id = -1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (argc != 2) {
        fprintf(stderr, "\n*** Usage: arraySum <inputFile>\n\n");
        exit(1);
    }
    
    t1 = MPI_Wtime();

    /* Master */
    if (id == 0) {
        // Read the array into memory
        readArray(argv[1], &masterArray, &howMany); 

        printf("Master has read the array of size %d\n\n", howMany);

        // Wait until the array has been read before we do anything
        MPI_Bcast(&howMany, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    /* Worker */
    else {
        // Need our workers to be patient
        MPI_Bcast(&howMany, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    
    finalSum = parallelSumArray(masterArray, howMany, id, numProc);

    t2 = MPI_Wtime();

    if (id == 0) {
        printf("\nThe sum of the values in the input file '%s' is %g\n",
                argv[1], finalSum);
        printf("It took '%g' seconds to compute\n", t2 - t1);
    }

    MPI_Finalize();
    
    return 0;
}

/* readArray fills an array with values from a file.
 * Receive: fileName, a char*,
 *          a, the address of a pointer to an array,
 *          n, the address of an int.
 * PRE: fileName contains N, followed by N double values.
 * POST: a points to a dynamically allocated array
 *        containing the N values from fileName
 *        and n == N.
 */

void readArray(char * fileName, double ** a, int * n) {
  int count, howMany;
  double * tempA;
  FILE * fin;

  fin = fopen(fileName, "r");
  if (fin == NULL) {
    fprintf(stderr, "\n*** Unable to open input file '%s'\n\n",
                     fileName);
    exit(1);
  }

  fscanf(fin, "%d", &howMany);
  tempA = calloc(howMany, sizeof(double));
  if (tempA == NULL) {
    fprintf(stderr, "\n*** Unable to allocate %d-length array",
                     howMany);
    exit(1);
  }

  for (count = 0; count < howMany; count++)
   fscanf(fin, "%lf", &tempA[count]);

  fclose(fin);

  *n = howMany;
  *a = tempA;
}

/* sumArray sums the values in an array of doubles.
 * Receive: a, a pointer to the head of an array;
 *          numValues, the number of values in the array.
 * Return: the sum of the values in the array.
 */

double sumArray(double * a, int numValues) {
  int i;
  double result = 0.0;

  for (i = 0; i < numValues; i++) {
    result += *a;
    a++;
  }

  return result;
}

double parallelSumArray(double *masterArray, int howMany, int id, int numProc) {

    // Get the number of elements per process
    int elements_per_proc = howMany / numProc;
    if (debug) { 
        printf("Process %d has elements  %d\n", id, elements_per_proc);
    }
    
    // Allocate our local array in each t hread
    double * localArray = malloc(sizeof(double) * elements_per_proc);
    
    // Scatter the array
    //  This applies to all of the processes.
    MPI_Scatter(masterArray, elements_per_proc, MPI_DOUBLE, localArray, elements_per_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (debug) {
        printf("Process %d has completed the scatter\n", id);
    }

    double localSum = sumArray(localArray, elements_per_proc);
    if (debug) {
        printf("The sum of the values in process '%d' is %g\n",    
                id, localSum);
    }

    free(localArray);

    double * local_sums = NULL;
    if (id == 0) {
        local_sums = malloc(sizeof(double) * numProc);
    }

    // Gather the results of the arrays
    MPI_Gather(&localSum, 1, MPI_DOUBLE, local_sums, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double finalSum = 0;

    if (id == 0) {
        finalSum = sumArray(local_sums, numProc);
    }
    
    return finalSum;
}

