/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for a simple registration program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: November 18, 2008
   >>>> Modifications: December 5, 2008 - Rewritten to accommodate new image class object utilizing GDAL library
   >>>>                October 24, 2011 - Rewritten to be a Common utility
   >>>>                July 31, 2012 - Modified for version 1.59 of Common Image software.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "params/params.h"
#include <image/image.h>
#include "register.h"
#include <iostream>
#include <gdal_priv.h>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");

extern Image baseImage;
extern Image registerImage;

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
    cout << "For help information: register -h or register -help" << endl << endl;
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
    status = register_image();
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "register parameter_file_name" << endl << endl;
    cout << "For help information: register -h or register -help" << endl;
    cout << "For version information: register -v or register -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The register progam is called as follows:" << endl;
    cout << endl << "register parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: register -h or register -help" << endl;
    cout << endl << "For version information: register -v or register -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-base_image		(string)	Formatted input base image (required)\n"
"-register_image		(string)	Formatted input image to register to\n"
"					base (required)\n"
"-OUT_registered_image	(string)	Output registered image (required)\n");

    cout << endl << "The output registered image will be formatted with the same format" << endl;
    cout << "as the input base image." << endl;

    return ;
}
