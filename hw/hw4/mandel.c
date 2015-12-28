/* Compute/draw mandelbrot set using MPI/MPE commands

 * Simplified Winter 2002, Joel Adams.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <mpi/mpi.h>
#include <mpi/mpe.h>
#include "display.h"

// globals
int verbose = 1;

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


int master_io(int size, int WINDOW_SIZE) 
{
    MPI_Status status;

    /*
    MPI_Request requests[size];

    int i = 0;
    for (i = 0; i < size; i++ ) {
        MPI_Request request;
        requests[i] = request;
    }
    */

    int ix = 0;
    int j = 1;
    short final[WINDOW_SIZE*WINDOW_SIZE];

    while (ix < WINDOW_SIZE - size) {
        for(j = 1; j < size; j++ ) {
            // if (requests[j] == MPI_SUCCESS) {
                // send(buf, count, type, dest, tag, comm)
                // send(index, 1, int, next available pe, 0, comm)
                // MPI_Isend(&ix, 1, MPI_INT, j, 0, MPI_COMM_WORLD, NULL);
                MPI_Send(&ix, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                ix = ix + 1;
            // }
        //}
    
        //for (j = 1; j < size; j++ ) {
            // if (requests[j] == MPI_SUCCESS) {
                // recv(buf, count, type, source, tag, comm, status)
                // MPI_Irecv(&final, WINDOW_SIZE, MPI_SHORT, 1, 0, MPI_COMM_WORLD, &requests[j]);
                short temp[WINDOW_SIZE];
                MPI_Recv(&temp, WINDOW_SIZE, MPI_SHORT, j, j, MPI_COMM_WORLD, &status);
                
                int index;
                for (index = 0; index < WINDOW_SIZE; index++) {
                    final[(ix-1) * WINDOW_SIZE + index] = temp[index];
                }
                
                printf("Received a value from process %d\n", j);
                fflush(stdout);
            // }
        }
    }

    ix = -1;
    for ( j = 1; j < size; j++) {
        printf("Clean up time");
        MPI_Send(&ix, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
    }

    // Draw the graph
    // Initialize the graph. Only done on the main id process
    MPE_XGraph graph;
    MPE_Open_graphics( &graph, MPI_COMM_WORLD, getDisplay(), -1, -1, WINDOW_SIZE, WINDOW_SIZE, 0);

    // Begin to print out the graph
    int iy;
    for(ix = 0; ix < WINDOW_SIZE; ix++) {
        // Cycle through the matrix rows.
        for(iy = 0; iy < WINDOW_SIZE; iy++) {
            // Print out an array
            if(final[iy + ix * WINDOW_SIZE] == 0){
                MPE_Draw_point(graph, ix, iy, MPE_RED);
            }
            else{
                MPE_Draw_point(graph, ix, iy, MPE_BLACK);
            }
        }
    }
    printf("\nClick in the window to continue...\n");

    int button;
    MPE_Get_mouse_press(graph, &ix, &iy, &button );
    MPE_Close_graphics( &graph );

    return 0;
}

/* This is the slave */
int slave_io(int rank, int WINDOW_SIZE)
{
    int        n,
               ix,
               iy;
    double     spacing=.005,
               x,
               y,
               c_real,
               c_imag,
               x_center = 1.0,
               y_center = 0.0;

    MPI_Status status;

    // Need a while loop to be continuously waiting for all of the tags to be finished.
    int tag = 0;
    
    // Get some debug statements to see what is happening with our process
    if (verbose) { printf("Tag before receive for process %d is %d\n", rank, tag); }
    // MPI_Irecv(&tag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
    MPI_Recv(&tag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    if (verbose) { printf("Tag after  receive for process %d is %d\n", rank, tag); }
    fflush(stdout);

    // As long as we don't have an invalid tag
    while(tag != -1 )
    {
        ix = tag;
        short child_array[WINDOW_SIZE];
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
                child_array[iy] = 0;
            }
            else{
            // 1 for black
                child_array[iy] = 1;
            }
        }
        // MPI_Isend(&child_array, WINDOW_SIZE, MPI_SHORT, 0, tag, MPI_COMM_WORLD, &request);
        MPI_Send(&child_array, WINDOW_SIZE, MPI_SHORT, 0, rank, MPI_COMM_WORLD);
        
        if (verbose) { printf("Tag before receive for process %d is %d\n", rank, tag); }
        // MPI_Irecv(&tag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        MPI_Recv(&tag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        if (verbose) { printf("Tag after  receive for process %d is %d\n", rank, tag); }
    }

    if (verbose) { printf("Process %d is complete\n", rank); }
    return 0;
}


int main(int argc, char* argv[])
{
    const int  WINDOW_SIZE = 64;


    int id = -1;
    int numProc = -1;

    // Initialize our MPI items
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (numProc < 2) {
        printf("Please enter 2 or more threads for a master worker setup\n");
        return 0;
    }
   
    if (id == 0) {
        master_io(numProc, WINDOW_SIZE);
    } else {
        slave_io(id, WINDOW_SIZE);
    }

    MPI_Finalize();
    return 0;
}

