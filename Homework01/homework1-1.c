/* homework1-1.c modified "circuitSatifiability.c" given by
 *  our teacher to achieve his goal.
 *
 *   This program is compiled with mpicc.
 *
 *   Using MPI_Reduce is DISALLOWED in this program.
 *
 *   Modified by 3Ution-BK, Oct. 2017.
 */
 
/*
 * circuitSatifiability.c solves the Circuit Satisfiability
 *  Problem using a brute-force sequential solution.
 *
 *   The particular circuit being tested is "wired" into the
 *   logic of function 'checkCircuit'. All combinations of
 *   inputs that satisfy the circuit are printed.
 *
 *   16-bit version by Michael J. Quinn, Sept 2002.
 *   Extended to 32 bits by Joel C. Adams, Sept 2013.
 */

#include <limits.h>    // UINT_MAX
#include <stdio.h>     // printf()

#include <mpi.h>

int checkCircuit(int, long);

int main (int argc, char *argv[])
{
  // Basic Variable
  long   i;                 // loop variable (64 bits)
  int    count = 0;         // number of solutions

  // MPI Variable (basic)
  int    my_rank;           // my current rank
  int    comm_size;         // number of process
  int    my_count = 0;

  // MPI Variable (tree-structured process)
  int    step_iter;         // while loop
  int    recv_count;        // receive total numbers of solutions from other
                            // process

  //MPI Variable (time)
  double start_t, total_t;  // start and total time of process

  
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // Record start time
  if (my_rank == 0)
  {
    start_t = MPI_Wtime();
  }

  // Calculate the circuit
  for (i = my_rank; i <= UINT_MAX; i += comm_size)
  {
    my_count += checkCircuit (my_rank, i);
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
        MPI_Recv(&recv_count,
                 1,
                 MPI_INT,
                 my_rank + step_iter / 2 + 1,
                 1,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        my_count += recv_count;
      }
      else if(my_rank > step_iter / 2)
      {
        MPI_Send(&my_count,
                 1,
                 MPI_INT,
                 my_rank - (step_iter / 2) - 1,
                 1,
                 MPI_COMM_WORLD);
      }
    }
    else
    {
      if(my_rank < step_iter / 2)
      {
        MPI_Recv(&recv_count,
                 1,
                 MPI_INT,
                 my_rank + step_iter / 2,
                 1,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        my_count += recv_count;
      }
      else if(my_rank >= step_iter / 2)
      {                
        MPI_Send(&my_count,
                 1,
                 MPI_INT,
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
    count = my_count;
	  printf("\nA total of %d solutions were found.\n\n", count);
    fflush(stdout);

    total_t = MPI_Wtime() - start_t;
    printf("Calculation finished in time %f secs.\n", total_t);
    fflush(stdout);
  }
   
  MPI_Finalize();
  return 0;
}

/* EXTRACT_BIT is a macro that extracts the ith bit of number n.
 *
 * parameters: n, a number;
 *             i, the position of the bit we want to know.
 *
 * return: 1 if 'i'th bit of 'n' is 1; 0 otherwise 
 */

#define EXTRACT_BIT(n,i) ((n & (1<<i)) ? 1 : 0)


/* checkCircuit() checks the circuit for a given input.
 * parameters: my_rank, the my_rank of the process checking;
 *             bits, the (long) rep. of the input being checked.
 *
 * output: the binary rep. of bits if the circuit outputs 1
 * return: 1 if the circuit outputs 1; 0 otherwise.
 */

#define SIZE 32

int checkCircuit(int my_rank, long bits) {
  int v[SIZE];        /* Each element is a bit of bits */
  int i;

  for (i = 0; i < SIZE; i++)
  {
    v[i] = EXTRACT_BIT(bits,i);
  }

  if (((v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3] ) &&
       (!v[3] || !v[4]) && (v[4] || !v[5]) && (v[5] || !v[6]) &&
       (v[5] || v[6]) && (v[6] || !v[15]) && (v[7] || !v[8]) &&
       (!v[7] || !v[13]) && (v[8] || v[9]) && (v[8] || !v[9]) &&
       (!v[9] || !v[10]) && (v[9] || v[11]) && (v[10] || v[11]) &&
       (v[12] || v[13]) && (v[13] || !v[14]) && (v[14] || v[15])) ||
      ((v[16] || v[17]) && (!v[17] || !v[19]) && (v[18] || v[19]) &&
       (!v[19] || !v[20]) && (v[20] || !v[21]) && (v[21] || !v[22]) &&
       (v[21] || v[22]) && (v[22] || !v[31]) && (v[23] || !v[24]) &&
       (!v[23] || !v[29]) && (v[24] || v[25]) && (v[24] || !v[25]) &&
       (!v[25] || !v[26]) && (v[25] || v[27]) && (v[26] || v[27]) &&
       (v[28] || v[29]) && (v[29] || !v[30]) && (v[30] || v[31])))
  {
    printf("%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
           "%d%d \n",
           my_rank,
           v[31],v[30],v[29],v[28],v[27],v[26],v[25],v[24],v[23],v[22],
           v[21],v[20],v[19],v[18],v[17],v[16],v[15],v[14],v[13],v[12],
           v[11],v[10],v[9],v[8],v[7],v[6],v[5],v[4],v[3],v[2],v[1],v[0]);
    fflush (stdout);
    
    return 1;
  } 
  else
  {
    return 0;
  }
}