Parallel Programming In C For Multiprocessor - Homework 2
===

[HackMD version](https://hackmd.io/MwUwxgRiFmC0wwA4AmcAsECc8IEZgB2OPFJAMyQEMQUqBWANhCA=)

## Question 1
**(Please use MPI_Scatterv and MPI_Gatherv to implement this homework)**

In Digital Image Processing, "Image smoothing" is a method that can reduce noise in an image or make a less pixelate image. One way is to replace this pixel with the average value of its neighbors.

**Link here:** [_Mean Filter_](https://homepages.inf.ed.ac.uk/rbf/HIPR2/mean.htm)

If this pixel is at the edge of the image, it will calculate the average with its neighbors and the opposite pixel.
Now we have a program that smooths the image. This program read a **24 bit BMP** file, which stored its pixel data in a matrix:

![pixel matrix](https://i.imgur.com/3Kya2rs.png)

In each pixel:
*   R stands for **Red** data
*   G stands for **Green** data
*   B stands for **Blue** data
*   And each data type is char. That means, every pixel is _3_ bytes(If char is 1 byte).

Every time we smooths the image, each pixel will calculate the average with its neighbor.

**Your job is to make this program parallelized and print out the work time.**

### Note
>   `bmp.h` is given by my teacher. I only edit `homewrok2-1.cpp`

>   **Compile**
>   ```mpicxx -o homework2-1 homewrok2-1.cpp```
>
>   **Execute**
>   ```mpiexe -n (number of process) ./homework2-1```

>   [_Image smoothing time chart (different number of smoothing time)_](https://live.amcharts.com/Y0Y2J/)
>
>   [_Image smoothing time chart (different number of process)_](https://live.amcharts.com/ZDcxZ/)

## Question 2
Parallel odd-even sort starts with n/common_size keys assigned to each process. It ends with all the keys stored on process 0 in sorted order.
**Write a program that implements parallel odd-even sort.**
Process 0 should read in n and broadcast it to the other processes. Each process should use a random number generator to create a local list of  n/common_size ints. Each process should then sort its local list, and process 0 should gather and print the local lists. Then the processes should merge the global list onto process 0, which prints the result.


>   Before I pass the local list into root process, I use`qsort()` to sort the array.

>   **Compile**
>   ```mpicc -o homework2-2 homewrok2-2.c```
>
>   **Execute**
>   ```mpiexe -n (number of process) ./homework2-2```

###### tags: `MPI` `parallel programming` `multiprocessor`
