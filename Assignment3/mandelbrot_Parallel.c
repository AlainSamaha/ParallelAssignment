#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

struct complex {
    double real;
    double imag;
};

int cal_pixel(struct complex c) {
    double z_real = 0;
    double z_imag = 0;
    double z_real2, z_imag2, lengthsq;
    int iter = 0;
    do {
        z_real2 = z_real * z_real;
        z_imag2 = z_imag * z_imag;
        z_imag = 2 * z_real * z_imag + c.imag;
        z_real = z_real2 - z_imag2 + c.real;
        lengthsq = z_real2 + z_imag2;
        iter++;
    } while ((iter < MAX_ITER) && (lengthsq < 4.0));
    return iter;
}

void save_pgm(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE* pgmimg; 
    pgmimg = fopen(filename, "wb"); 
    fprintf(pgmimg, "P2\n"); 
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT);  
    fprintf(pgmimg, "255\n");  
    for (int i = 0; i < HEIGHT; i++) { 
        for (int j = 0; j < WIDTH; j++) { 
            fprintf(pgmimg, "%d ", image[i][j]); 
        } 
        fprintf(pgmimg, "\n"); 
    } 
    fclose(pgmimg); 
}

int main() {
    int image[HEIGHT][WIDTH];
    double AVG = 0;
    int N = 10;
    double total_time[N];
    struct complex c;

    //this is the loop to run the mandelbrot set calculation multiple times (10 times)
    for (int k = 0; k < N; k++) {
        double start_time = omp_get_wtime();

        //using openmp to parallelize the outer loop the approach I am using is row-wise chunking
        //each thread gets a chunck of data to process (chunck of rows)
        #pragma omp parallel for schedule(dynamic)
        //the goal is to have load balancing so I used schedule(dynamic) to achive that
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
                c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
                image[i][j] = cal_pixel(c);
            }
        }

        double end_time = omp_get_wtime();
        total_time[k] = end_time - start_time;
        printf("Execution time of trial [%d]: %f seconds\n", k, total_time[k]);
        AVG += total_time[k];
    }

    save_pgm("mandelbrot.pgm", image);
    printf("The average execution time of 10 trials is: %f ms\n", (AVG / N) * 1000);

    return 0;
}