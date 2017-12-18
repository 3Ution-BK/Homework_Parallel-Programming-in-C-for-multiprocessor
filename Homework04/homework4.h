#ifndef HOMEWORK4_H
#define HOMEWORK4_H

#include <cstdlib>
#include <pthread.h>

#include "bmp.h"

// Constant

// File open success
static const bool kFileOpenSuccess = true;
// File open failure
static const bool kFileOpenFailure = false;
// Number of smoothpProcess
static const std::size_t kNSmooth = 1000;
// Input file
static const char *file_name_input = "input.bmp";
// Output file
static const char *file_name_output = "output1.bmp";

// bmp file
// Header of the bmp file
BMPHEADER bmp_header;
// Infomation of the bmp file
BMPINFO bmp_info;
// The bmp pixel data of the bmp file
RGBTRIPLE **bmp_data = 0;
// Temporary storage of the bmp file
RGBTRIPLE **bmp_temp_data = 0;

// pthread
// Pthread barrier
pthread_barrier_t pthread_barrier;
// Check whether the bmp swap is finished
bool is_swap_finish = false;
// Mutex for critical section
pthread_mutex_t swap_mutex = PTHREAD_MUTEX_INITIALIZER;
// Data for passing information to individual pthread
struct pthread_data_t {
  // Start location for local pthread
  std::size_t local_bmp_height_start;
  // Finish location for local pthread
  std::size_t local_bmp_height_end;
};

// Function

// ReadBMP is a function that read the bmp file.
//
// parameters: const char *fileName, the name of the file.
//
// global variable: BMPHEADER bmp_header, Header of the file;
//                  BMPINFO bmp_info, Infomation of the file;
//                  RGBTRIPLE **bmp_data, The bmp pixel data of the file.
//
// error: if file is not readable, print out necessary info
//
// return: kFileOpenFailure if file is not readable, otherwise kFileOpenSuccess
bool ReadBMP(const char *fileName);

// SaveBMP is a function that write the bmp file.
//
// parameters: const char *fileName, the name of the file.
//
// global variable: BMPHEADER bmp_header, Header of the file;
//                  BMPINFO bmp_info, Infomation of the file;
//                  RGBTRIPLE **bmp_data, The bmp pixel data of the file.
//
// error: if file is not readable, print out necessary info
//
// return: kFileOpenFailure if file is not readable, otherwise kFileOpenSuccess
bool SaveBMP(const char *fileName);

// New2DMemory is a function that allocate memory to 2D array.
//
// parameters: std::size_t height, height of the 2D array;
//             std::size_t width, witdh of the 2D array;
//
// return: 2D array(pointer)
RGBTRIPLE **New2DMemory(std::size_t height, std::size_t width);

// Delete2DMemory is a function that delete 2D array memory.
//
// parameters: RGBTRIPLE **memory, pointer of the 2D array;
//
// return: none.
void Delete2DMemory(RGBTRIPLE **memory);

// PixelSmoothing() returns the pixel aften smoothing.
//
// parameters: RGBTRIPLE &ans, a pointer point to one pixel in the image
//             RGBTRIPLE &a ~ &e, a pointer point to one pixel in the image
//
// return: none.
void PixelSmoothing(RGBTRIPLE &ans, RGBTRIPLE &a, RGBTRIPLE &b, RGBTRIPLE &c,
               RGBTRIPLE &d, RGBTRIPLE &e);

// ImageSmoothing() is the pointer for pthread to smooths the image.
//
// parameters: RGBTRIPLE &ans, a pointer point to one pixel in the image
//             RGBTRIPLE &a ~ &e, a pointer point to one pixel in the image
//
// return: none.
void *ImageSmoothing(void *data);

#endif //HOMEWORK4_H
