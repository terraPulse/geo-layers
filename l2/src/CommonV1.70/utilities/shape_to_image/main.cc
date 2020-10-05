/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for shape to image program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: January 9, 2012
   >>>> Modifications: July 31, 2012 - Modified for version 1.59 of Common Image software.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "shape_to_image.h"
#include "params/params.h"
#include <image/image.h>
#include <shape/shape.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <gdal_priv.h>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");
Image maskImage;
Shape shapeFile;

// Forward function declarations
void usage();
void help();

int main(int argc, char *argv[])
{
  bool status = true;

  GDALAllRegister();

  if (argc == 1)
  {
    usage();
    status = false;
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: shape_to_image -h or shape_to_image -help" << endl << endl;
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
    params.print();
    status = shape_to_image();
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "shape_to_image parameter_file_name" << endl << endl;
    cout << "For help information: shape_to_image -h or shape_to_image -help" << endl;
    cout << "For version information: shape_to_image -v or shape_to_image -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The shape_to_image progam is called as follows:" << endl;
    cout << endl << "shape_to_image parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: shape_to_image -h or shape_to_image -help" << endl;
    cout << endl << "For version information: shape_to_image -v or shape_to_image -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-shape_file		(string)	Input shapefile (required)\n"
"-mask_image		(string)	Mask image (required -\n"
"					in any GDAL recognized image format)\n"
"-output_image		(string)	Output image (required - will be same\n"
"					format as mask_image)\n");

    cout << endl << "The mask_image supplies the area of interest over which" << endl;
    cout << "and the spatial resolution at which the shape_file information" << endl;
    cout << "is plotted in the output image." << endl;

    return ;
}

