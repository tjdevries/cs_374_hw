/* arraySum.c uses an array to sum the values in an input file,
 *  whose name is specified on the command-line.
 * Joel Adams, Fall 2005
 * for CS 374 (HPC) at Calvin College.
 * Adapted by:
 *      Ryan Siekman
 */

#include <stdio.h>      /* I/O stuff */
#include <stdlib.h>     /* calloc, etc. */
#include <mpi/mpi.h>

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;
double parallelsumArray(double * arr, int num, int id, int numProc);

int main(int argc, char * argv[])
{
  int  howMany;
  double totalSum, start = 0, end = 0;
  double * a;
  int id = -1, numProcesses = -1;
  double Total, IO, t1, t2;

  if (argc != 2) {
    fprintf(stderr, "\n*** Usage: arraySum <inputFile>\n\n");
    exit(1);
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
  t1 = MPI_Wtime();
  if(id == 0){
      start = MPI_Wtime();
      readArray(argv[1], &a, &howMany);
      end = MPI_Wtime();
      IO = end - start;
  }
  if(numProcesses == 1){ totalSum = sumArray(a, howMany); }
  else{
    totalSum = parallelsumArray(a, howMany, id, numProcesses);
  }
  t2 = MPI_Wtime();
  Total = t2 - t1;
  if(id == 0){
    printf("%f\n%f\n", IO, Total);
  //  printf("The sum of the values in the input file '%s is %g\n'", argv[1], totalSum);
    free(a);
  }
  
  MPI_Finalize();

  return 0;
}

double parallelsumArray(double * arr, int num, int id, int numProc){
  int root = 0;
  double sum, totalSum, t3, t4, t5, t6, scatter, s;
  if(id == 0){ //master reads array and scatters value
    MPI_Bcast(&num, 1, MPI_INT, root, MPI_COMM_WORLD);
    //printf("%i\t%g\n", numProc, *arr);
  }
  else{ //workers sum their part of the array
    MPI_Bcast(&num, 1, MPI_INT, root, MPI_COMM_WORLD);
  }
  
  int numElements = num/numProc;
  double* localA = malloc(sizeof(double) * numElements);
  
  //if(id == 0){
  t3 = MPI_Wtime();//}
  MPI_Scatter(arr, numElements, MPI_DOUBLE, localA, numElements, MPI_DOUBLE, root, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  t4 = MPI_Wtime();
  if(id == 0){ scatter = t4 - t3;}
  t5 = MPI_Wtime();
  sum = sumArray(localA, numElements);
  MPI_Reduce(&sum, &totalSum, numProc, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  t6 = MPI_Wtime();
  s = t6 - t5; 
  if( id == 0){  
      //s = t6 - t5;
      printf("New Timing\n%g\n%g\n", scatter, s);
  }
  return totalSum;
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

