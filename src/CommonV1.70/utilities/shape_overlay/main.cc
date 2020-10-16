/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for shape overlay program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: April 5, 2010
   >>>> Modifications: May 13, 2011 - Modified to work with image with no geometric information. Assumes (0,0) origin.
   >>>>                May 13, 2011 - Reduced to just one input shapefile.
   >>>>                July 31, 2012 - Modified for version 1.59 of Common Image software.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "shape_overlay.h"
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
Image inputImage;

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
    cout << "For help information: shape_overlay -h or shape_overlay -help" << endl << endl;
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
    params.set_temp_files();
    status = shape_overlay();
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
    cout << "shape_overlay parameter_file_name" << endl << endl;
    cout << "For help information: shape_overlay -h or shape_overlay -help" << endl;
    cout << "For version information: shape_overlay -v or shape_overlay -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The shape_overlay progam is called as follows:" << endl;
    cout << endl << "shape_overlay parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: shape_overlay -h or shape_overlay -help" << endl;
    cout << endl << "For version information: shape_overlay -v or shape_overlay -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-input_image		(string)	Input image (required -\n"
"					in any GDAL recognized image format)\n"
"-shape_file		(string)	Input shapefile (required)\n"
"-RGB		(integer triplet)	Red, Green, Blue values for\n"
"					shape overlay for shape_file,\n"
"					comma delimited (required)\n"
"-output_image		(string)	Output image (required - ENVI format)\n");

    cout << endl << "The output image will be the input image with the shape file data overlayed." << endl;

    return ;
}

