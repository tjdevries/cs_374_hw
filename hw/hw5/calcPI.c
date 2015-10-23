/* calcPi.c calculates PI using the integral of the unit circle.
 * Since the area of the unit circle is PI, 
 *  PI = 4 * the area of a quarter of the unit circle 
 *    (i.e., its integral from 0.0 to 1.0)
 *
 * Joel Adams, Fall 2013 for CS 374 at Calvin College.
 */

#include "integral.h"   // integrate()
#include <stdio.h>      // printf(), etc.
#include <stdlib.h>     // exit()
#include <math.h>       // sqrt() 
#include <mpi/mpi.h>

/* function for unit circle (x^2 + y^2 = 1)
 * parameter: x, a double
 * return: sqrt(1 - x^2)
 */
double f(double x) {
   return sqrt(1.0 - x*x);
}

/* retrieve desired number of trapezoids from commandline arguments
 * parameters: argc: the argument count
 *             argv: the argument vector
 * return: the number of trapezoids to be used.
 */            
unsigned long long processCommandLine(int argc, char** argv) {
   if (argc == 1) {
       return 1;
   } else if (argc == 2) {
//       return atoi( argv[1] );
       return strtoull( argv[1], 0, 10 );
   } else {
       fprintf(stderr, "Usage: ./calcPI [numTrapezoids]");
       exit(1);
   }
}
 

int main(int argc, char** argv) {
    long double approximatePI = 0;
    long double finalPI;
    const long double REFERENCE_PI = 3.141592653589793238462643383279L;

    int numProc = -1;
    int id = -1;

    // Start our MPI items
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
   
    unsigned long long numTrapezoids = processCommandLine(argc, argv); 

    // Our values should be between 0.0 and 1.0.
    //  We will assign each PE it's own section of that section
    long double width = 1.0 / numProc;
   
    long double start = id * width;
    long double end = (id + 1) * width;

    printf("Process %d has (start, end): (%Lf, %LF)\n", id, start, end);
    fflush(stdout);

    approximatePI = integrateTrap(id * width, (id + 1) * width, numTrapezoids) * 4.0;

    printf("Using %llu trapezoids, the approximate found for process %d are PI are:\n%.30Lf\n",
           numTrapezoids, id, approximatePI);

    MPI_Reduce(&approximatePI, &finalPI, 1, MPI_LONG_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (id == 0) {
        printf("Final approximation for Pi vs reference:\n%.30Lf\n%.30Lf\n", finalPI, REFERENCE_PI);
    }

    MPI_Finalize();

    return 0;
}

