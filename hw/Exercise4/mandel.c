/* Compute/draw mandelbrot set using MPI/MPE commands

 * Simplified Winter 2002, Joel Adams.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi/mpi.h>
#include <mpi/mpe.h>
#include "display.h"

/* compute the mandelbrot-set function for a given
 *  point in the complex plane.
 * Receive: doubles x and y,
 *          complex c.
 * Modify: doubles ans_x and ans_y.
 * POST: ans_x and ans_y contain the results of the mandelbrot-set
 *        function for x, y, and c.
 */
void compute(double x, double y, double c_real, double c_imag,
        double *ans_x, double *ans_y)
{
        *ans_x = x*x - y*y + c_real;
        *ans_y = 2*x*y + c_imag;
}

/* compute the 'distance' between x and y.
 * Receive: doubles x and y.
 * Return: x^2 + y^2.
 */
double distance(double x, double y)
{
        return(x*x + y*y);
}


int main(int argc, char* argv[])
{
    const int  WINDOW_SIZE = 1024;

    int verbose = 1;

    int        n,
               ix,
               iy,
               button,
               id = 0,
               numProc,
               chunk = 0;
    double     spacing=.005,
               x,
               y,
               c_real,
               c_imag,
               x_center = 1.0,
               y_center = 0.0,
               starttime;

    // Initialize our MPI items
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    if(id == 0){starttime = MPI_Wtime();}
    // Set the size of our chunks.
    chunk = WINDOW_SIZE / numProc;

    // Do some debug printing
    if (verbose) { printf("Chunk size set to %d. Total size is %d", chunk, chunk * numProc); }

    // Each process creates its own local chunkarray
    short chunkarray[(chunk * WINDOW_SIZE)];
    int end = (id +1 ) * chunk;

    // Loop through and calculate the MPI items
    int start = id * chunk;
    // Make sure that we get all the values, instead of missing one in case of
    //  none perfect division.
    if (verbose) { printf("Process %d has start: %d, end: %d\n", id, start, end); }

    // Loop through and calculate the values.
    for (ix = start; ix < end; ix++)
    {
        for (iy = 0; iy <  WINDOW_SIZE; iy++)
        {
            c_real = (ix - 400) * spacing - x_center;
            c_imag = (iy - 400) * spacing - y_center;
            x = y = 0.0;
            n = 0;
            while (n < 50 && distance(x,y) < 4.0)
            {
                 compute(x,y,c_real,c_imag,&x,&y);
                 n++;
            }

            if (n < 50) {
            // 0 for red
            //  Because these are the local arrays, they should be 0 relative.
            //  When we gather them, they will be appended to each other.
            //  So that is why we have ix - start.
                chunkarray[iy + (ix - start) * WINDOW_SIZE] = 0;
            }
            else{
            // 1 for black
                chunkarray[iy + (ix - start) * WINDOW_SIZE] = 1;
            }
        }
    }
    // Put all the values from the chunk array into our final array, that will be used for graphing

    // Make the array a square of window size by window size
    short array[WINDOW_SIZE * WINDOW_SIZE];

    // MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
    //  void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)

    MPI_Gather(&chunkarray, chunk*WINDOW_SIZE, MPI_SHORT, &array, chunk*WINDOW_SIZE, MPI_SHORT, 0, MPI_COMM_WORLD);

    if (id == 0) {
        double endtime = MPI_Wtime();
        double time = endtime - starttime;
        printf("It took %fl seconds\n", time);
        // Initialize the graph. Only done on the main id process
        MPE_XGraph graph;
        MPE_Open_graphics( &graph, MPI_COMM_WORLD, getDisplay(), -1, -1, WINDOW_SIZE, WINDOW_SIZE, 0);

        // Begin to print out the graph
        for(ix = 0; ix < WINDOW_SIZE; ix++) {
            // Cycle through the matrix rows.
            for(iy = 0; iy < WINDOW_SIZE; iy++) {
                // Print out an array
                if(array[iy + ix * WINDOW_SIZE] == 0){
                    MPE_Draw_point(graph, ix, iy, MPE_RED);
                }
                else{
                    MPE_Draw_point(graph, ix, iy, MPE_BLACK);
                }
            }
        }
        printf("\nClick in the window to continue...\n");
        MPE_Get_mouse_press(graph, &ix, &iy, &button );
        MPE_Close_graphics( &graph );
    }
    MPI_Finalize();
    return 0;
}

