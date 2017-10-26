/* homework1-2.c solves the parameter pi by using "Monte
 *  Carlo" method and record the time.
 *
 *   This program is compiled with mpicc.
 *
 *   Using MPI_Reduce is DISALLOWED in this program.
 *
 *   Made by 3Ution-BK, Oct. 2017.
 */

#include <stdio.h>  // printf(), scanf()
#include <stdlib.h> // srand()
 
#include <mpi.h>    // basic mpi functions

/* RANDOM_NUMBER() is a macro that return a random double value
 *  between -1 and 1.
 *
 * parameters: None.
 *
 * output: a double value between -1 and 1. 
 * return: a double value between -1 and 1.
 */

#define RANDOM_NUMBER() (((double)rand() / ((double)RAND_MAX / 2.0)) - 1.0)

int main(void)
{
  // Basic Variable
  long long int num_of_toss;        // total number of tosses
  long long int toss_iter;          // for loop
  double        x, y;               // value from the origin
  double        distance_squared;   // distance of the origin (squared)
  long long int num_in_cir = 0;     // total numbers of tosses in circle
  double        pi_estimate;        // estimate value of pi
    
  // MPI Variable (basic)
  int           comm_size;          // number of process
  int           my_rank;            // my current rank
  long long int my_num_in_cir = 0;  // my total number of tosses in circle

  // MPI Variable (tree-structured process)
  int           step_iter;          // while loop
  long long int recv_num_in_cir;    // receive total numbers of tosses in
                                    // circle from other process

  //MPI Variable (time)
  double        start_t, total_t;   // start and total time of process
    

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
       
  srand(time(NULL));

  // Read the total number of tosses from user
  if(my_rank == 0)
  {
    printf("Enter the number of tosses: ");
    fflush(stdout);
    scanf("%d", &num_of_toss);
    
    start_t = MPI_Wtime();
  }    

  // Broadcast the number of tosses to every process
  MPI_Bcast(&num_of_toss,
            1,
            MPI_LONG_LONG_INT,
            0,
            MPI_COMM_WORLD);

  // Calculate the number of tosses in circle
  for(toss_iter = 0 + (long long int) my_rank;
      toss_iter < num_of_toss;
      toss_iter += (long long int) comm_size)
  {
    x = RANDOM_NUMBER();
    y = RANDOM_NUMBER();
    distance_squared = x * x + y * y;
        
    if(distance_squared < 1.0)
    {
      ++my_num_in_cir;
    }
  }
    
  // Send back value by using tree-structured communication
  //  (Page 47 in Chapter 3)
  //
  //   In each step, split the processes in half. The former process receives
  //   the "my_count" value from the latter one. Then devide the "step_iter"
  //   in half. Repeat this step untilthe value of "step_iter" is 1.
  //
  //   "step_iter" records the number of the proceess needs to send or receive
  //   the value.
  step_iter = comm_size;
    
  while(step_iter != 1 && my_rank < step_iter)
  {
    // Check the remainder of the "step_iter" and uses different code in
    // order to perform properly.
    if(step_iter % 2 == 1)
    {
      if(my_rank < step_iter / 2)
      {
         MPI_Recv(&recv_num_in_cir,
                  1,
                  MPI_LONG_LONG_INT,
                  my_rank + step_iter / 2 + 1,
                  1,
                  MPI_COMM_WORLD,
                  MPI_STATUS_IGNORE);

         my_num_in_cir += recv_num_in_cir;
      }
      else if(my_rank > step_iter / 2)
      {
        MPI_Send(&my_num_in_cir,
                  1,
                  MPI_LONG_LONG_INT,
                  my_rank - (step_iter / 2) - 1,
                  1,
                  MPI_COMM_WORLD);
      }
    }
    else
    {
      if(my_rank < step_iter / 2)
      {
        MPI_Recv(&recv_num_in_cir,
                 1,
                 MPI_LONG_LONG_INT,
                 my_rank + step_iter / 2,
                 1,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        my_num_in_cir += recv_num_in_cir;
      }
      else if(my_rank >= step_iter / 2)
      {                
        MPI_Send(&my_num_in_cir,
                 1,
                 MPI_LONG_LONG_INT,
                 my_rank - (step_iter / 2),
                 1,
                 MPI_COMM_WORLD);
      }
    }

    step_iter = step_iter / 2 + ((step_iter % 2 == 1) ? 1 : 0);
  }
    
  // Print out the result
  if(my_rank == 0)
  {
    num_in_cir = my_num_in_cir;

    pi_estimate = 4.0 * ((double) num_in_cir) / ((double) num_of_toss);
    printf("Estimate pi value: %f\n", pi_estimate);
    fflush(stdout);
        
    total_t = MPI_Wtime() - start_t;
    printf("Time(sec): %f\n", total_t);
    fflush(stdout);
  }
    
  MPI_Finalize();
  return 0;
}