/* Compute/draw mandelbrot set using MPI/MPE commands
 * Written Winter, 1998, W. David Laverell.
 * Simplified Winter 2002, Joel Adams. 
 */

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
{  printf("here");
    const int  WINDOW_SIZE = 1024;

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
               *array[WINDOW_SIZE][WINDOW_SIZE];
    MPE_XGraph graph;
    MPE_Open_graphics( &graph, MPI_COMM_WORLD, getDisplay(), -1, -1, WINDOW_SIZE, WINDOW_SIZE, 0);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    chunk = WINDOW_SIZE / numProc;
    /*
       printf("\nEnter spacing (.005): "); fflush(stdout);
       scanf("%lf",&spacing);
       printf("\nEnter coordinates of center point (0,0): "); fflush(stdout);
       scanf("%lf %lf", &x_center, &y_center);
       printf("\nSpacing=%lf, center=(%lf,%lf)\n",
       spacing, x_center, y_center);
       */
    printf("here2");
    double chunkarray[chunk][WINDOW_SIZE];
    for (ix = (id)*chunk; ix < (id+1)*chunk; ix++)
    {printf("here3");
       for (iy = 0; iy <  WINDOW_SIZE; iy++)
       {
         c_real=(ix - 400) * spacing - x_center;
         c_imag=(iy - 400) * spacing - y_center;
         x = y = 0.0;
         n = 0;
         while (n < 50 && distance(x,y) < 6.0)
         {
            compute(x,y,c_real,c_imag,&x,&y);
            n++;}

         if(n<50){
            chunkarray[ix][iy] = 0;}
         else{ 
             chunkarray[ix][iy] = 1;}

         //MPI_Gather(&x, chunk, MPI_DOUBLE, &array[ix][iy], chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);  
        
        /*if (n < 50)
          MPE_Draw_point(graph,ix,iy,MPE_RED);
        else
          MPE_Draw_point(graph,ix,iy,MPE_BLACK);*/
       }
    }
    MPI_Gather(chunkarray, chunk, MPI_DOUBLE, array[ix][iy], chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // pause until mouse-click so the program doesn't terminate
    if (id == 0) {
        for(ix = 0; ix < WINDOW_SIZE; ix++){
            for(iy = 0; iy<WINDOW_SIZE;iy++){
                 if(array[ix][iy] == 0){
                    MPE_Draw_point(graph,ix,iy,MPE_RED);}
                 else{
                    MPE_Draw_point(graph,ix,iy,MPE_BLACK);}
            }
        }
        printf("\nClick in the window to continue...\n");
        MPE_Get_mouse_press( graph, &ix, &iy, &button );
    }

    MPE_Close_graphics( &graph );
    MPI_Finalize();
    return 0;
}
