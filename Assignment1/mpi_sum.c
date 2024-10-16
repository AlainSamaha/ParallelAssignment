#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 100000  // Define the size of the large array

int main(int argc, char** argv) {
    int rank, size;
    float *array = NULL;
    float partial_sum = 0.0, total_sum = 0.0;
    float *local_array = NULL;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    
    // Get the rank (ID) of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Allocate memory for the large array in rank 0 only
    if (rank == 0) {
        array = (float*) malloc(N * sizeof(float));
        
        // Seed the random number generator and fill the array with random values
        srand(time(NULL));  
        for (int i = 0; i < N; i++) {
            array[i] = ((float)rand() / (float)(RAND_MAX)) * 100.0;  // Random float values between 0 and 100
        }
    }

    // Determine the size of each subarray
    int local_size = N / size;

    // Allocate memory for the local subarray
    local_array = (float*) malloc(local_size * sizeof(float));

    //start timing
    double start_time = MPI_Wtime();

    // Scatter the large array into subarrays
    MPI_Scatter(array, local_size, MPI_FLOAT, local_array, local_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Each process calculates the sum of its local array
    for (int i = 0; i < local_size; i++) {
        partial_sum += local_array[i];
    }

    // Each process prints its rank and its partial sum
    printf("Process %d, Partial Sum: %f\n", rank, partial_sum);

    // Reduce all partial sums into the total sum at rank 0
    MPI_Reduce(&partial_sum, &total_sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();


    if (rank == 0) {
        
        double end_time = MPI_Wtime();
        printf("Execution time: %f seconds\n", end_time - start_time);
        free(array);  // Free the large array memory in rank 0
    }

   // Free the local array memory in all ranks
    free(local_array);

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

