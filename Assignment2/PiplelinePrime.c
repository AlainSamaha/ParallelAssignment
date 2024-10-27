#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define TERMINATOR -1

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            printf("Usage: mpirun -np <number_of_processes> ./prime2 <n>\n");
            printf("Where <n> is the upper limit for checking primes.\n");
        }
        MPI_Finalize();
        return 1;
    }

    //getting n from the command line argument
    int n = atoi(argv[1]);
    if (rank == 0) {
        printf("Starting prime generation up to %d\n", n);
    }

    double start_time = MPI_Wtime();

    //process 0 generates the primes and sends to other processes
    if (rank == 0) {
        //first we send 2 since it is the first prime
        MPI_Send(&(int){2}, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        //we eleminate the even since they cannot be prime to make the program efficient 
        for (int num = 3; num <= n; num += 2) {
            MPI_Send(&num, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }

        MPI_Send(&(int){TERMINATOR}, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        printf("Generated and sent all numbers\n");
    } else {
        int number;
        int x = 0; // Variable to hold the prime for this process
        int is_first = 1;  // Flag to identify first prime assignment

        while (1) {
            //we recive the primes from the previous process P-1
            MPI_Recv(&number, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);

            //if the termi is recieved terminate the process
            if (number == TERMINATOR) {
                if (rank < size - 1) {
                    MPI_Send(&number, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                }
                break;
            }

            if (is_first) {
                x = number;
                is_first = 0;
            } else if (number % x != 0) { //checking if divisible
                printf("Prime found: %d\n", number);
                //sending the prime found to P+1
                if (rank < size - 1) {
                    MPI_Send(&number, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                }
            }
        }

        if (x != 0) {
            printf("Prime found: %d\n", x);
        }
    }

    double end_time = MPI_Wtime();

    //printing the execution only once 
    if (rank == 0) {
        printf("Total execution time: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
