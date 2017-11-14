/* homework2-1.cpp modified "Smooth.cpp" given by
 *  our teacher to achieve his goal.
 *
 *   This program is compiled with mpicxx.
 *
 *   Modified by 3Ution-BK, Oct. 2017.
 */

#include "bmp.h"

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>
#include <string>

// Constant
static const int kFileOpenSuccess = 1; // File open success
static const int kFileOpenFailure = 0; // File open failure

static const int kNSmooth = 1000; // Number of smoothpProcess

static const int kProcessRoot = 0; // Process root rank

// Global Variable
BMPHEADER bmpHeader;         // Header of the file
BMPINFO bmpInfo;             // Infomation of the file
RGBTRIPLE **BMPSaveData = 0; // The bmp pixel data of the file

// Function
int ReadBMP(const char *fileName);       // read file
int SaveBMP(const char *fileName);       // save file
RGBTRIPLE **New2DMemory(int Y, int X);   // allocate 2D memory
void Delete2DMemory(RGBTRIPLE **memory); // delete 2D memory
void Smoothing(RGBTRIPLE &ans,
               RGBTRIPLE &a, RGBTRIPLE &b, RGBTRIPLE &c, RGBTRIPLE &d,
               RGBTRIPLE &e);            // smoothing image

int main(int argc, char *argv[])
{
  // Basic Variable
  const char *infileName = "input.bmp";    // Input file
  const char *outfileName = "output1.bmp"; // Output file

  // MPI Variable (basic)
  int local_rank; // my current rank
  int comm_size;  // number of process

  int *global_bmp_size = 0;          // Array: record bmp size for each process
  int *global_bmp_displacements = 0; // Array: record bmp displacements for
                                     // each process

  // MPI Variable (local bmp data)
  int local_bmp_height;                   // Height of the local_bmp
  RGBTRIPLE **local_bmp_data = NULL;      // temporary storage: local bmp data
  RGBTRIPLE **local_bmp_save_data = NULL; // local bmp data

  // MPI Variable (communication temporary storage)
  RGBTRIPLE **local_bmp_upper_temp = NULL; // upper storage
  RGBTRIPLE **local_bmp_lower_temp = NULL; // lower storage

  // MPI Type
  MPI_Datatype type_mpi_rgbtriple; // MPI type: RGBTRIPLE

  // MPI Variable (time)
  double start_time, end_time; // Start and total time of process

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

  // Create MPI Type
  MPI_Type_contiguous(3, MPI_UNSIGNED_CHAR, &type_mpi_rgbtriple);
  MPI_Type_commit(&type_mpi_rgbtriple);

  // Read the file
  if (local_rank == kProcessRoot)
  {
    if (ReadBMP(infileName))
    {
      std::cout << "Read file successfully!!" << std::endl;
      fflush(stdout);
    }
    else
    {
      std::cout << "Read file fails!!" << std::endl;
      fflush(stdout);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  // Record Start time
  MPI_Barrier(MPI_COMM_WORLD);
  if (local_rank == kProcessRoot)
  {
    start_time = MPI_Wtime();
  }

  // Broadcast 'bmpInfo.biHeight' and 'bmpInfo.biWidth' to each process
  MPI_Bcast(&bmpInfo.biHeight, 1, MPI_INT, kProcessRoot, MPI_COMM_WORLD);
  MPI_Bcast(&bmpInfo.biWidth, 1, MPI_INT, kProcessRoot, MPI_COMM_WORLD);

  // Allocate memory for not-root process
  if (local_rank != kProcessRoot)
  {
    BMPSaveData = New2DMemory(bmpInfo.biHeight, bmpInfo.biWidth);
  }

  // Calculate the attributes of the scatter pieces of each process
  global_bmp_size = new int[comm_size];
  global_bmp_displacements = new int[comm_size];

  local_bmp_height = (bmpInfo.biHeight / comm_size) +
                     ((bmpInfo.biHeight % comm_size >= local_rank + 1) ?
                      1 : 0);
  int local_bmp_size = bmpInfo.biWidth * local_bmp_height;

  // Gather the attributes of the scatter pieces of each process
  MPI_Gather(&local_bmp_size, 1, MPI_INT, // Gather item
             global_bmp_size, 1, MPI_INT, // Destination
             kProcessRoot,
             MPI_COMM_WORLD);

  // Calculate displacements
  if (local_rank == kProcessRoot)
  {
    for (size_t i = 0; i < comm_size; ++i)
    {
      global_bmp_displacements[i] = ((i != kProcessRoot) ?
                                     (global_bmp_displacements[i - 1] +
                                        global_bmp_size[i - 1]) :
                                     0);
    }
  }

  // Allocate the local bmp data and communication temporary storage
  local_bmp_data = New2DMemory((bmpInfo.biHeight / comm_size) + 1,
                               bmpInfo.biWidth);
  local_bmp_save_data = New2DMemory((bmpInfo.biHeight / comm_size) + 1,
                                    bmpInfo.biWidth);

  local_bmp_upper_temp = New2DMemory(1, bmpInfo.biWidth);
  local_bmp_lower_temp = New2DMemory(1, bmpInfo.biWidth);

  // Scatterv the picture to each process
  MPI_Scatterv(*BMPSaveData, global_bmp_size, global_bmp_displacements,
               type_mpi_rgbtriple, // Scatter item
               *local_bmp_save_data, local_bmp_size,
               type_mpi_rgbtriple, // Destination
               kProcessRoot,
               MPI_COMM_WORLD);

  // Image smoothing process
  for (int count = 0; count < kNSmooth; ++count)
  {
    // Process communicate

    // Determine the partner
    int left_partner = ((local_rank != 0) // Left Partner
                           ? (local_rank - 1) : (comm_size - 1));
    int right_partner = ((local_rank != comm_size - 1) // Right Partner
                            ? (local_rank + 1) : 0);

    // Send upper communication temporary storage
    MPI_Sendrecv(&local_bmp_save_data[local_bmp_height - 1][0],
                 bmpInfo.biWidth, type_mpi_rgbtriple, right_partner,
                 1, // Send to
                 &local_bmp_upper_temp[0][0],
                 bmpInfo.biWidth, type_mpi_rgbtriple, left_partner,
                 1, // Receive from
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    // Send lower communication temporary storage
    MPI_Sendrecv(&local_bmp_save_data[0][0], bmpInfo.biWidth,
                 type_mpi_rgbtriple, left_partner, 1, // Send to
                 &local_bmp_lower_temp[0][0], bmpInfo.biWidth,
                 type_mpi_rgbtriple, right_partner, 1, // Receive from
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

    // Swap the local save data (namespace std)
    std::swap(local_bmp_save_data, local_bmp_data);

    // Image smoothing
    for (int i = 0; i < local_bmp_height; ++i)
    {
      for (int j = 0; j < bmpInfo.biWidth; ++j)
      {
        // Calculate the direction
        int Top = ((i > 0) ? (i - 1) : (local_bmp_height - 1));
        int Down = ((i < local_bmp_height - 1) ? (i + 1) : 0);
        int Left = ((j > 0) ? (j - 1) : (bmpInfo.biWidth - 1));
        int Right = ((j < bmpInfo.biWidth - 1) ? (j + 1) : 0);

        // Calculate the Result
        if (i == 0)
        {
          // Top pixel
          Smoothing(local_bmp_save_data[i][j], // result
                    local_bmp_data[i][j], local_bmp_upper_temp[0][j],
                    local_bmp_data[Down][j], local_bmp_data[i][Left],
                    local_bmp_data[i][Right]);
        }
        else if (i == local_bmp_height - 1)
        {
          // Button pixel
          Smoothing(local_bmp_save_data[i][j], // result
                    local_bmp_data[i][j], local_bmp_data[Top][j],
                    local_bmp_lower_temp[0][j], local_bmp_data[i][Left],
                     local_bmp_data[i][Right]);

        }
        else
        {
          // Middle pixel
          Smoothing(local_bmp_save_data[i][j], // result
                    local_bmp_data[i][j], local_bmp_data[Top][j],
                    local_bmp_data[Down][j], local_bmp_data[i][Left],
                    local_bmp_data[i][Right]);
        }
      }
    }
  }

  // Gatherv the picture to process root
  MPI_Gatherv(*local_bmp_save_data, local_bmp_size,
              type_mpi_rgbtriple, // Gather item
              *BMPSaveData, global_bmp_size, global_bmp_displacements,
              type_mpi_rgbtriple, // Destination
              kProcessRoot,
              MPI_COMM_WORLD);

  // Record the total time and print out
  MPI_Barrier(MPI_COMM_WORLD);
  if (local_rank == kProcessRoot)
  {
    end_time = MPI_Wtime();
    std::cout << "The execution time = " << (end_time - start_time) << std::endl;
  }

  // Save the data
  if (local_rank == kProcessRoot)
  {
    if (SaveBMP(outfileName))
    {
      std::cout << "Save file successfully!!" << std::endl;
      fflush(stdout);
    }
    else
    {
      std::cout << "Save file fails!!" << std::endl;
      fflush(stdout);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  // Delete the file
  Delete2DMemory(BMPSaveData);
  Delete2DMemory(local_bmp_data);
  Delete2DMemory(local_bmp_save_data);
  Delete2DMemory(local_bmp_upper_temp);
  Delete2DMemory(local_bmp_lower_temp);
  delete[] global_bmp_size;
  delete[] global_bmp_displacements;

  MPI_Type_free(&type_mpi_rgbtriple);
  MPI_Finalize();

  return 0;
}

// ReadBMP is a function that read the bmp file.
//
// parameters: const char *fileName, the name of the file.
//
// global variable: BMPHEADER bmpHeader, Header of the file;
//                  BMPINFO bmpInfo, Infomation of the file;
//                  RGBTRIPLE **BMPSaveData, The bmp pixel data of the file.
// error: if file is not readable, print out necessary info
// return: kFileOpenFailure if file is not readable, otherwise kFileOpenSuccess
int ReadBMP(const char *fileName)
{
  // Construct ifstream object
  std::ifstream bmpFile(fileName, std::ios::in | std::ios::binary);

  // If file cannot open
  if (!bmpFile)
  {
    std::cerr << "It can't open file!!" << std::endl;
    return kFileOpenFailure;
  }

  // Read file header
  bmpFile.read(reinterpret_cast<char*>(&bmpHeader), sizeof(BMPHEADER));

  // If file header is wrong
  if (bmpHeader.bfType != 0x4d42)
  {
    std::cerr << "This file is not .BMP!!" << std::endl;
    return kFileOpenFailure;
  }

  // Read file info
  bmpFile.read(reinterpret_cast<char*>(&bmpInfo), sizeof(BMPINFO));

  // If pixel is not 24 bits
  if (bmpInfo.biBitCount != 24)
  {
    std::cerr << "The file is not 24 bits!!" << std::endl;
    return kFileOpenFailure;
  }

  // Fixed the picture width to the multiples of 4
  while (bmpInfo.biWidth % 4 != 0)
  {
    bmpInfo.biWidth++;
  }

  // Allowcate the memory
  BMPSaveData = New2DMemory(bmpInfo.biHeight, bmpInfo.biWidth);

  // Read file pixel
  bmpFile.read(reinterpret_cast<char*>(BMPSaveData[0]),
               bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

  // Destruct ifstream object
  bmpFile.close();

  return kFileOpenSuccess;
}

// SaveBMP is a function that write the bmp file.
//
// parameters: const char *fileName, the name of the file.
//
// global variable: BMPHEADER bmpHeader, Header of the file;
//                  BMPINFO bmpInfo, Infomation of the file;
//                  RGBTRIPLE **BMPSaveData, The bmp pixel data of the file.
// error: if file is not readable, print out necessary info
// return: kFileOpenFailure if file is not readable, otherwise kFileOpenSuccess
int SaveBMP(const char *fileName)
{
  // If file header is wrong
  if (bmpHeader.bfType != 0x4d42)
  {
    std::cout << "This file is not .BMP!!" << std::endl;
    return kFileOpenFailure;
  }

  // Construct ofstream object
  std::ofstream newFile(fileName, std::ios::out | std::ios::binary);

  // If file cannot create
  if (!newFile)
  {
    std::cout << "The File can't create!!" << std::endl;
    return kFileOpenFailure;
  }

  // Write file header
  newFile.write(reinterpret_cast<char*>(&bmpHeader), sizeof(BMPHEADER));

  // Write file info
  newFile.write(reinterpret_cast<char*>(&bmpInfo), sizeof(BMPINFO));

  // Write file pixel
  newFile.write(reinterpret_cast<char*>(BMPSaveData[0]),
                bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

  // Destruct ofstream object
  newFile.close();

  return kFileOpenSuccess;
}

// New2DMemory is a function that allocate memory to 2D array.
//
// parameters: int Y, y of the 2D array;
//             int X, x of the 2D array;
//
// return: 2D array(pointer)
RGBTRIPLE **New2DMemory(int Y, int X)
{
  // Construct and initialize pointer
  RGBTRIPLE **temp = new RGBTRIPLE *[Y];
  RGBTRIPLE *temp2 = new RGBTRIPLE[Y * X];
  memset(temp, 0, sizeof(RGBTRIPLE) * Y);
  memset(temp2, 0, sizeof(RGBTRIPLE) * Y * X);

  // Connect the pointers
  for (int i = 0; i < Y; ++i)
  {
    temp[i] = &temp2[i * X];
  }

  return temp;
}

// Delete2DMemory is a function that delete 2D array memory.
//
// parameters: RGBTRIPLE **memory, pointer of the 2D array;
//
// return: none.
void Delete2DMemory(RGBTRIPLE **memory)
{
  // Construct and initialize pointer
  delete[] memory[0];
  delete[] memory;
}

// Smoothing() returns the value aften smoothing.
// parameters: RGBTRIPLE &ans, a pointer point to one pixel in the image
//             RGBTRIPLE &a ~ &e, a pointer point to one pixel in the image
//
// return: none.
void Smoothing(RGBTRIPLE &ans, RGBTRIPLE &a, RGBTRIPLE &b,
               RGBTRIPLE &c, RGBTRIPLE &d, RGBTRIPLE &e)
{
  ans.rgbBlue  = static_cast<double>(a.rgbBlue  + b.rgbBlue  + c.rgbBlue +
                                     d.rgbBlue  + e.rgbBlue)  / 5 + 0.5f;
  ans.rgbGreen = static_cast<double>(a.rgbGreen + b.rgbGreen + c.rgbGreen +
                                     d.rgbGreen + e.rgbGreen) / 5 + 0.5f;
  ans.rgbRed   = static_cast<double>(a.rgbRed   + b.rgbRed   + c.rgbRed +
                                     d.rgbRed   + e.rgbRed)   / 5 + 0.5f;
}
