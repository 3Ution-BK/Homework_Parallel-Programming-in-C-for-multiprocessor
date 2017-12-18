Parallel Programming In C For Multiprocessor - Homework 4
===

[HackMD version](https://hackmd.io/BwIwJgbBwGYgtAJgMbGPALAZgKboIYCsYM8+AjOQJzn5bUzkDsQA)

**(Please use POSIX Threads to implement this homework)**

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
>   Like homework02, we have to smooth the same picture. The only different things is to use pthread instead of MPICH.
>   `bmp.h` is given by my teacher. I only edit `homewrok4.cpp`
>   A interesting fact is that once the pthread number is more than the total cores in this cluster(in this cluster, 8), the more pthread is involved, the more time it will waste.

>   **Compile**
>   ```g++ -pthread -Wall -o homework4 homework4.cpp```
>
>   **Execute**
>   ```./homework4```

>   [_Image smoothing time chart (different number of smoothing time with 8 pthreads)_](https://live.amcharts.com/Tk1OT/)
>
>   [_Image smoothing time chart (different number of pthreads)_](https://live.amcharts.com/NkYzk/)
