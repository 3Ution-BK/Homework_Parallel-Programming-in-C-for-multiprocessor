// homework4.cpp modified "Smooth.cpp" given by our teacher to achieve his goal.
//  It uses pthread to do the Image smoothing process.
//
//   This program is compiled with gcc with POSIX system.
//
//   Modified by 3Ution-BK, Dec. 2017.

#include "homework4.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
  // Read the thread number, exit if user input value is equal to or less than 0
  std::size_t pthread_size;
  int number_user_input;

  std::cout << "Please enter how many pthread you want to create: ";
  std::cin >> number_user_input;

  if (number_user_input <= 0)
  {
    std::cout << number_user_input << " thread is not allowed" << std::endl;
    exit(EXIT_FAILURE);
  }
  else
  {
    pthread_size = static_cast<size_t>(number_user_input);
  }


  // Read and determine the file is bmp or not
  if (ReadBMP(file_name_input) == true)
  {
    std::cout << "Read file success" << std::endl;
    fflush(stdout);
  }
  else
  {
    std::cerr << "Error: Read file failed" << std::endl;
    fflush(stdout);
    exit(EXIT_FAILURE);
  }


  time_t start_time, end_time;
  time(&start_time);


  unsigned barrier_size = static_cast<unsigned>(pthread_size);
  pthread_barrier_init(&(pthread_barrier), /* normal attribute = */ NULL,
                       barrier_size);


  // Create pthread and assigned their work
  pthread_t pthread_ID[pthread_size];
  pthread_data_t pthread_data[pthread_size];
  std::size_t height_start = 0;
  for (std::size_t iter = 0; iter < pthread_size; ++iter)
  {
    // Check the boundaries of the current pthread
    std::size_t height_size = (bmp_info.biHeight / pthread_size) +
        ((bmp_info.biHeight % pthread_size >= iter + 1) ? 1 : 0);

    pthread_data[iter].local_bmp_height_start = height_start;
    pthread_data[iter].local_bmp_height_end   = height_start + height_size;


    int return_flag
        = pthread_create(&(pthread_ID[iter]), /* normal attribute = */ NULL,
                        ImageSmoothing,
                        reinterpret_cast<void *>(&(pthread_data[iter])));

    if (return_flag != 0)
    {
      // create failure
      std::cerr << "Error: create pthread failed" << std::endl
                << "with return value" << return_flag;

      exit(EXIT_FAILURE);
    }


    height_start = pthread_data[iter].local_bmp_height_end;
  }


  // Join the pthread
  for (std::size_t iter = 0; iter < pthread_size; ++iter)
  {
    int return_flag = pthread_join(pthread_ID[iter],
                                   /* pthread return status = */ NULL);

    if (return_flag != 0)
    {
      // join failure
      std::cerr << "Error: join pthread failed" << std::endl
                << "with return value" << return_flag;

      exit(EXIT_FAILURE);
    }
  }


  time(&end_time);
  std::cout << "The execution time = " << (end_time - start_time) << std::endl;


  if (SaveBMP(file_name_output) == true)
  {
    std::cout << "Save file success" << std::endl;
  }
  else
  {
    std::cerr << "Error: Save file failed" << std::endl;
  }


  return 0;
}

void *ImageSmoothing(void *data)
{
  pthread_data_t *pthread_data = reinterpret_cast<pthread_data_t *>(data);
  size_t local_bmp_width = bmp_info.biWidth;


  // Image smoothing process
  for (std::size_t count = 0; count < kNSmooth; count++)
  {
    // Using mutex to prevent multiple swapping
    // Use one "if" condition is to make sure that later pthread will bypass
    // this condition without wasting time queuing to enter critical section
    // if earlier pthread has finish swapping.
    if (is_swap_finish == false)
    {
      // Critical Section
      pthread_mutex_lock(&swap_mutex);
      if (is_swap_finish == false)
      {
        // Swap the bmp data (namespace std)
        std::swap(bmp_data, bmp_temp_data);
        is_swap_finish = true;
      }
      pthread_mutex_unlock(&swap_mutex);
    }


    pthread_barrier_wait(&pthread_barrier);


    // Smoothing process
    for (std::size_t i = pthread_data->local_bmp_height_start;
         i < pthread_data->local_bmp_height_end;
         ++i)
    {
      for (std::size_t j = 0; j < local_bmp_width; ++j)
      {
        // Calculate the direction
        std::size_t Top   = ((i > 0) ? (i - 1) : (bmp_info.biHeight - 1));
        std::size_t Down  = ((i < static_cast<size_t>(bmp_info.biHeight) - 1) ?
                             (i + 1) : 0);
        std::size_t Left  = ((j > 0) ? (j - 1) : (bmp_info.biWidth - 1));
        std::size_t Right = ((j < static_cast<size_t>(bmp_info.biWidth) - 1) ?
                             (j + 1) : 0);

        PixelSmoothing(/* Result */ bmp_data[i][j],
                       bmp_temp_data[i][j], bmp_temp_data[Top][j],
                       bmp_temp_data[Down][j], bmp_temp_data[i][Left],
                       bmp_temp_data[i][Right]);
      }
    }



    is_swap_finish = false;


    pthread_barrier_wait(&pthread_barrier);
  }

  return 0;
}

bool ReadBMP(const char *fileName)
{
  std::ifstream bmp_file(fileName, std::ios::in | std::ios::binary);
  if (!bmp_file)
  {
    std::cerr << "It can't open file!!" << std::endl;
    return kFileOpenFailure;
  }


  bmp_file.read(reinterpret_cast<char *>(&bmp_header), sizeof(BMPHEADER));
  if (bmp_header.bfType != 0x4d42)
  {
    std::cerr << "This file is not .BMP!!" << std::endl;
    return kFileOpenFailure;
  }


  bmp_file.read(reinterpret_cast<char *>(&bmp_info), sizeof(BMPINFO));
  if (bmp_info.biBitCount != 24)
  {
    std::cerr << "The file is not 24 bits!!" << std::endl;
    return kFileOpenFailure;
  }


  // Fixed the picture width to the multiples of 4
  while (bmp_info.biWidth % 4 != 0)
  {
    ++bmp_info.biWidth;
  }

  // Allowcate the memory
  bmp_data = New2DMemory(bmp_info.biHeight, bmp_info.biWidth);
  bmp_temp_data = New2DMemory(bmp_info.biHeight, bmp_info.biWidth);

  // Read file pixel into bmp_data
  bmp_file.read(reinterpret_cast<char *>(bmp_data[0]),
                sizeof(RGBTRIPLE) * bmp_info.biWidth * bmp_info.biHeight);


  bmp_file.close();

  return kFileOpenSuccess;
}

bool SaveBMP(const char *fileName)
{
  if (bmp_header.bfType != 0x4d42)
  {
    std::cerr << "This file is not .BMP!!" << std::endl;
    return kFileOpenFailure;
  }


  std::ofstream newFile(fileName, std::ios::out | std::ios::binary);
  if (!newFile)
  {
    std::cerr << "The File can't create!!" << std::endl;
    return kFileOpenFailure;
  }


  newFile.write(reinterpret_cast<char *>(&bmp_header), sizeof(BMPHEADER));
  newFile.write(reinterpret_cast<char *>(&bmp_info), sizeof(BMPINFO));
  newFile.write(reinterpret_cast<char *>(bmp_data[0]),
                sizeof(RGBTRIPLE) * bmp_info.biWidth * bmp_info.biHeight);


  newFile.close();


  Delete2DMemory(bmp_temp_data);
  Delete2DMemory(bmp_data);

  return kFileOpenSuccess;
}

RGBTRIPLE **New2DMemory(std::size_t height, std::size_t width)
{
  RGBTRIPLE **temp = new RGBTRIPLE *[height];
  RGBTRIPLE *temp2 = new RGBTRIPLE[height * width];
  memset(temp,  /* value */ 0, sizeof(RGBTRIPLE) * height);
  memset(temp2, /* value */ 0, sizeof(RGBTRIPLE) * height * width);


  for (std::size_t i = 0; i < height; ++i)
  {
    temp[i] = &temp2[i * width];
  }

  return temp;
}

void Delete2DMemory(RGBTRIPLE **memory)
{
  delete[] memory[0];
  delete[] memory;
}

void PixelSmoothing(RGBTRIPLE &ans, RGBTRIPLE &a, RGBTRIPLE &b,
                    RGBTRIPLE &c, RGBTRIPLE &d, RGBTRIPLE &e)
{
  ans.rgbBlue  = static_cast<double>(a.rgbBlue + b.rgbBlue + c.rgbBlue +
                                     d.rgbBlue + e.rgbBlue)
                 / 5 + 0.5f;
  ans.rgbGreen = static_cast<double>(a.rgbGreen + b.rgbGreen + c.rgbGreen +
                                     d.rgbGreen + e.rgbGreen)
                 / 5 + 0.5f;
  ans.rgbRed   = static_cast<double>(a.rgbRed + b.rgbRed + c.rgbRed +
                                     d.rgbRed + e.rgbRed)
                 / 5 + 0.5f;
}
