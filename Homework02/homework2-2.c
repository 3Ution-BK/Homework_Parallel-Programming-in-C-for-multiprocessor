/* homework2-2.c shows the parallel odd-even sort algorithm and record the time.
 *
 *   This program is compiled with mpicc.
 *
 *   Made by 3Ution-BK, Oct. 2017.
 */
#include <mpi.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* Constant  */
enum num_limit {MAX_NUM = 20};
enum boolean {FALSE = 0, TRUE = 1};
enum process {PROCESS_ROOT = 0};
enum mergeArray {MERGE_SMALL = 0, MERGE_LARGE = 1};

/* Functions  */
int arrayCompare(const void *a, const void *b); /* compare two value  */
void swap(int *a, int *b);                      /* swap two value  */
void mergeArray(int *local_array, int local_array_size,
                int *recv_array, int recv_array_size,
                enum mergeArray state);         /* merge two array  */

int main(void)
{
  /* Basic Variables  */
  int total_num; /* Total number of the array  */

  /* Global Array  */
  int *global_num_array = NULL;              /* Global array  */
  int *global_num_array_displacement = NULL; /* Displacement of each process
                                                 in global array  */
  int *global_num_array_size = NULL;         /* Array size of each process in
                                                 global array  */

  /* MPI Variable (basic)  */
  int comm_size;  /* number of process  */
  int local_rank; /* local current rank  */

  /* MPI Variable (local Array)  */
  int local_remainder_check;  /* Checks whether the current process need to
                                  add one more value in the array or not  */
  int *local_num_array;       /* local number array */
  int local_num_array_size;   /* size of the local number array */
  int *recv_num_array = NULL; /* number array of the recviver */
  int recv_num_array_size;    /* size of the recviver's number array */

  /* MPI Variable (time)  */
  double start_time, total_time; /* Start and total time of process  */

  /* Loop Iter */
  size_t loop_iter; /* For loop iterator  */


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

  /* User input the 'total_num'  */
  if (local_rank == PROCESS_ROOT)
  {
    char input_str[MAX_NUM]; /* Input String, used for input  */

    printf("Please enter how many number you want to calculate:");
    fflush(stdout);

    fgets(input_str, MAX_NUM, stdin);
    total_num = strtol(input_str, NULL, 10);

    start_time = MPI_Wtime();
  }

  /* Random seed reset  */
  srand(local_rank * 100);

  /* Broadcast 'total_num' to each process  */
  MPI_Bcast(&total_num, 1, MPI_INT, PROCESS_ROOT, MPI_COMM_WORLD);

  /* Check the exact size of the array and create them  */
  local_remainder_check = (((total_num % comm_size) >= (local_rank + 1))
                           ? 1 : 0);
  local_num_array_size = total_num / comm_size + local_remainder_check;

  local_num_array = (int *)malloc(sizeof(int) * (total_num / comm_size + 1));
  recv_num_array = (int *)malloc(sizeof(int) * (total_num / comm_size + 1));
  /* malloc two exact size array, then use boundary check
          ('local_num_array_size' and 'recv_num_array_size') to avoid unwanted
          value  */

  /* Create a list of number at random  */
  for (loop_iter = 0; loop_iter < local_num_array_size; ++loop_iter)
  {
    local_num_array[loop_iter] = rand() % UINT_MAX;
  }

  /* Using qsort to sort local array  */
  qsort(local_num_array, local_num_array_size, sizeof(int),
        arrayCompare);

  /* Create the array for process root  */
  if (local_rank == PROCESS_ROOT)
  {
    global_num_array = (int *)malloc(sizeof(int) * total_num);
    global_num_array_size = (int *)malloc(sizeof(int) * comm_size);
    global_num_array_displacement = (int *)malloc(sizeof(int) * comm_size);
  }

  /* Gather the array length into 'global_num_array_size'  */
  MPI_Gather(&local_num_array_size, 1, MPI_INT,
             global_num_array_size, 1, MPI_INT, PROCESS_ROOT,
             MPI_COMM_WORLD);

  /* Check the displacement of each array  */
  if (local_rank == PROCESS_ROOT)
  {
    global_num_array_displacement[0] = 0;

    for (loop_iter = 1; loop_iter < comm_size; ++loop_iter)
    {
      global_num_array_displacement[loop_iter]
          = global_num_array_displacement[loop_iter - 1] +
            global_num_array_size[loop_iter - 1];
    }
  }

  /* Gather the number array  */
  MPI_Gatherv(local_num_array, local_num_array_size, MPI_INT,
              global_num_array, global_num_array_size,
              global_num_array_displacement, MPI_INT, PROCESS_ROOT,
              MPI_COMM_WORLD);

  /* Print out the array  */
  if (local_rank == PROCESS_ROOT)
  {
    int output_rank = -1;  /* Used for print out local rank  */
    size_t no_counter = 1; /* Number counter. USed for print out position of
                               the value in local array  */

    for (loop_iter = 0; loop_iter < total_num; ++loop_iter)
    {
      if (loop_iter == global_num_array_displacement[output_rank + 1])
      {
        no_counter = 1;
        ++output_rank;
        printf("process %d local array:\n", output_rank);
        fflush(stdout);
      }

      printf("Rank: %3d || No. %5d || %d\n",
             output_rank, no_counter, global_num_array[loop_iter]);
             /* rank, position, and the value in the local array  */
      fflush(stdout);
      ++no_counter;
    }
  }

  /* Sort the array  */
  /* Uses the odd even sort, the maximum phase is the number of the process  */
  for (loop_iter = 0; loop_iter < comm_size; ++loop_iter)
  {
    int local_partner; /* The partner of the local process  */

    /* Find the partner of the current phase  */
    if (loop_iter % 2 == 1)
    {
      /* Odd phase  */
      local_partner = local_rank + ((local_rank % 2 == 0) ? -1 : 1);
    }
    else
    {
      /* Even phase  */
      local_partner = local_rank + ((local_rank % 2 == 0) ? 1 : -1);
    }

    /* If the local process has no partner of the partner is invalid, skip the
        current process immediately  */
    if ((local_partner < 0) || (local_partner >= comm_size))
    {
      continue;
    }

    /* If the local processor the partner has no array value, skip the current
        process immediately(or the sendrecv process will fail)  */
    if ((local_rank >= total_num) || (local_partner >= total_num))
    {
      continue;
    }

    /* Communicate the partner and avoiding the deadlock  */
    if (local_rank % 2 == 0)
    {
      /* First check the size of the array  */
      MPI_Send(&local_num_array_size,
               1,
               MPI_INT,
               local_partner,
               0,
               MPI_COMM_WORLD);
      MPI_Recv(&recv_num_array_size,
               1,
               MPI_INT,
               local_partner,
               MPI_ANY_TAG,
               MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

      /* If one of the array size is 0, then there is no data to send or
          receive. if that case happened, skip it  */
      if (local_num_array_size == 0 || recv_num_array_size == 0)
      {
        break;
      }

      /* After that, pass the whole array  */
      MPI_Send(local_num_array,
               local_num_array_size,
               MPI_INT,
               local_partner,
               0,
               MPI_COMM_WORLD);
      MPI_Recv(recv_num_array,
               recv_num_array_size,
               MPI_INT,
               local_partner,
               MPI_ANY_TAG,
               MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }
    else
    {
      /* First check the size of the array  */
      MPI_Recv(&recv_num_array_size,
               1,
               MPI_INT,
               local_partner,
               MPI_ANY_TAG,
               MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      MPI_Send(&local_num_array_size,
               1,
               MPI_INT,
               local_partner,
               0,
               MPI_COMM_WORLD);

      /* If one of the array size is 0, then there is no data to send or
          receive. if that case happened, skip it  */
      if (local_num_array_size == 0 || recv_num_array_size == 0)
      {
        break;
      }

      /* After that, pass the whole array  */
      MPI_Recv(recv_num_array,
               recv_num_array_size,
               MPI_INT,
               local_partner,
               MPI_ANY_TAG,
               MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      MPI_Send(local_num_array,
               local_num_array_size,
               MPI_INT,
               local_partner,
               0,
               MPI_COMM_WORLD);
    }

    /* Store the needed value in local array. The smaller rank stored the
        smaller value while the larger rank stored the larger value  */
    if (local_rank < local_partner)
    {
      mergeArray(local_num_array, local_num_array_size,
                 recv_num_array, recv_num_array_size,
                 MERGE_SMALL);
    }
    else
    {
      mergeArray(local_num_array, local_num_array_size,
                 recv_num_array, recv_num_array_size,
                 MERGE_LARGE);
    }
  }

  /* Odd even sort finished  */

  /* Gather the sorted number array  */
  MPI_Gatherv(local_num_array, local_num_array_size, MPI_INT,
              global_num_array, global_num_array_size,
              global_num_array_displacement, MPI_INT, PROCESS_ROOT,
              MPI_COMM_WORLD);

  /* Print out the final array  */
  if (local_rank == PROCESS_ROOT)
  {
    printf("Final Result:\n");

    for (loop_iter = 0; loop_iter < total_num; ++loop_iter)
    {
      printf("No. %5d || %d\n", loop_iter + 1, global_num_array[loop_iter]);
      fflush(stdout);
    }

    total_time = MPI_Wtime() - start_time;
    printf("Total Time(sec): %f\n", total_time);
  }

  free(global_num_array);
  free(global_num_array_size);
  free(local_num_array);
  free(recv_num_array);
  MPI_Finalize();
  return 0;
}

/* arrayCompare() compares two value. Use this to sort the array in assending
 *  order.
 * parameters: a, a value;
 *             b, a value.
 *
 * return: if the return value < 0, a goes before b.
 *         if the return value = 0, a is equivalent to b.
 *         if the return value > 0, a goes after b.
 */
int arrayCompare(const void *a, const void *b)
{
  if (*(int *)a <  *(int *)b) {return -1;}
  if (*(int *)a == *(int *)b) {return  0;}
  if (*(int *)a >  *(int *)b) {return  1;}
}

/* swap() swap the location of the two value
 * parameters: int *a, a pointer point to the value
 *             int *b, a pointer point to the value
 *
 * return: none.
 */
void swap(int *a, int *b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

/* mergeArray() merge two array into one
 * parameters: int *local_array, local array;
 *             int local_array_size, local array size;
 *             int *recv_array, receiver array;
 *             int recv_array_size, receiver array size;
 *             enum mergeArray state, merge rule.
 *
 * return: none.
 */
void mergeArray(int *local_array, int local_array_size,
                int *recv_array, int recv_array_size,
                enum mergeArray state)
{
  /* Variable  */
  int return_array[local_array_size]; /* return_array  */
  size_t local_iter, recv_iter; /* Iterator  */
  int return_iter; /* Iterator  */

  /* Determine the state  */
  if (state == MERGE_SMALL)
  {
    /* Initialize the iterator  */
    local_iter = recv_iter = 0;
    return_iter = 0;
    /* Merge two array into one  */
    while (return_iter < local_array_size)
    {
      if (local_array[local_iter] <= recv_array[recv_iter])
      {
        return_array[return_iter] = local_array[local_iter];
        ++return_iter;
        ++local_iter;
      }
      else
      {
        return_array[return_iter] = recv_array[recv_iter];
        ++return_iter;
        ++recv_iter;
      }
    }
  }
  else
  {
    /* Initialize the iterator  */
    local_iter = local_array_size - 1;
    recv_iter = recv_array_size - 1;
    return_iter = local_array_size - 1;

    /* Merge two array into one  */
    while (return_iter >= 0)
    {
      if (local_array[local_iter] > recv_array[recv_iter])
      {
        return_array[return_iter] = local_array[local_iter];
        --return_iter;
        --local_iter;
      }
      else
      {
        return_array[return_iter] = recv_array[recv_iter];
        --return_iter;
        --recv_iter;
      }
    }
  }

  /* Put it back to local array  */
  for (local_iter = 0; local_iter < local_array_size; ++local_iter)
  {
    local_array[local_iter] = return_array[local_iter];
  }

  return;
}
