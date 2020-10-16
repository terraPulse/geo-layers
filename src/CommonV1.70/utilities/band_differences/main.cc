/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for a program that generates a complete set of normalized band differences
   >>>>         for an input image.
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: November 17, 2011
   >>>> Modifications: July 23, 2012 - Modified for version 1.59 of Common Image software.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "band_differences.h"
#include "params/params.h"
#include <image/image.h>
#ifdef GDAL
#include <gdal_priv.h>
#endif
#include <iostream>
#include <cstring>
#include <climits>
#include <cstdlib>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");

#ifdef GDAL
extern Image inputImage;
#endif

// Forward function declarations
void usage();
void help();

int main(int argc, char *argv[])
{
  bool status = true;

#ifdef GDAL
  GDALAllRegister();
#endif

  if (argc == 1)
  {
    usage();
    status = false;
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: band_differences -h or band_differences -help" << endl << endl;
    status = false;
  }
  else if ((strncmp(argv[1],"-h",2) == 0) || (strncmp(argv[1],"-help",5) == 0))
  {
    help();
    status = false;
  }
  else if (strncmp(argv[1],"-",1) == 0)
  {
    usage();
    cout << "The parameter file name cannot start with an \"-\"" << endl;
    status = false;
  }
  else
  {
    if (argc != 2)
    {
      usage();
      status = false;
    }
    else
    {
      status = false;
      status = params.read(argv[1]);
    }
  }

  if (status)
  {  
    params.set_temp_files();
    params.print();
    status = band_differences();
    params.remove_temp_files();
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "band_differences parameter_file_name" << endl << endl;
    cout << "For help information: band_differences -h or band_differences -help" << endl;
    cout << "For version information: band_differences -v or band_differences -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The band_differences progam is called as follows:" << endl;
    cout << endl << "band_differences parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: band_differences -h or band_differences -help" << endl;
    cout << endl << "For version information: band_differences -v or band_differences -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-input_image		(string)	Input image (required)\n"
#ifndef GDAL
"-ncols			(int)		Number of columns in input image data\n"
"				        (0 < ncols < %d, required)\n"
"-nrows			(int)		Number of rows in input image data\n"
"				        (0 < nrows < %d, required)\n"
"-nbands			(int)		Number of spectral bands in input image\n"
"					data.  (0 < nbands < %d, required)\n"
"-dtype			(string)	Input image data type. Currently must\n"
"					be either:\n"
"					 UInt8   => \"unsigned char (8-bit)\" or\n"
"					 UInt16  => \"unsigned short int\n"
"						     (16-bit)\" or\n"
"					 Float32 => \"float (32-bit)\"\n"
"					(required)\n",USHRT_MAX,USHRT_MAX,USHRT_MAX);
  fprintf(stdout,"-mask_image		(string)	Input data mask file name\n"
#else
"-mask_image		(string)	Input data mask file name\n"
#endif
#ifndef GDAL
"					Data type must be unsigned char.\n"
#endif
"					(optional, default = {none})\n"
#ifndef GDAL
"-mask_value		(int)		If input data mask file is provided,\n"
"					this is the value in the mask file that\n"
"					designates bad data\n"
"					(optional, default = 0.)\n"
#endif
"-band_difference_images	(string)	Base name for set of output band\n"
"					difference images (required)\n");
#ifdef GDAL
    cout << endl << "A set of output band differences image will be output in ENVI format." << endl;
#else
    cout << endl << "A set of output band differences image will be output in \"raw\" format." << endl;
#endif
    return ;
}
