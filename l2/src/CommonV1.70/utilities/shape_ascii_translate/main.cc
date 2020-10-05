/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for shape_ascii_translate program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: December 1, 2011
   >>>> Modifications: December 18, 2011 - Upgraded to also handle Polygon and PolygonZ shape file types
   >>>>                January 3, 2012 - Upgraded to properly handle multipart Polygon and Arc shapes
   >>>>                July 31, 2012 - Modified for version 1.59 of Common Image software.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "shape_ascii_translate.h"
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
    cout << "For help information: shape_ascii_translate -h or shape_ascii_translate -help" << endl << endl;
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
    status = shape_ascii_translate();
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "shape_ascii_translate parameter_file_name" << endl << endl;
    cout << "For help information: shape_ascii_translate -h or shape_ascii_translate -help" << endl;
    cout << "For version information: shape_ascii_translate -v or shape_ascii_translate -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The shape_ascii_translate progam is called as follows:" << endl;
    cout << endl << "shape_ascii_translate parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: shape_ascii_translate -h or shape_ascii_translate -help" << endl;
    cout << endl << "For version information: shape_ascii_translate -v or shape_ascii_translate -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters must be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-mask_image		(string)	Input mask image. This mask\n"
"					image should have a non-zero value\n"
"					at valid pixel locations and a zero\n"
"					value and invalid pixel locations (in\n"
"					any GDAL recognized image format)\n"
"-shape_file_in		(string)	Input shapefile (in ESRI shapefile format)\n"
"-ascii_shape_out	(string)	Output ASCII format shapefile\n");

    fprintf(stdout,"\nThe following parameter may also be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-field_names	(string)	Field Names used in translation\n"
"					(comma delimited list, optional)\n");

    return ;
}

