/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for shape_info program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: April 14, 2010
   >>>> Modifications: May 20, 2011 - Added MULTIPOINT
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "shape_info.h"
#include "params/params.h"
#include <shape/shape.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");
Shape shapeInFile;

// Forward function declarations
void usage();
void help();

int main(int argc, char *argv[])
{
  bool status = true;

  if (argc == 1)
  {
    usage();
    status = false;
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: shape_info -h or shape_info -help" << endl << endl;
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
//    params.print();
    status = shape_info();
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "shape_info parameter_file_name" << endl << endl;
    cout << "For help information: shape_info -h or shape_info -help" << endl;
    cout << "For version information: shape_info -v or shape_info -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The shape_info progam is called as follows:" << endl;
    cout << endl << "shape_info parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: shape_info -h or shape_info -help" << endl;
    cout << endl << "For version information: shape_info -v or shape_info -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-shape_in_file		(string)	Input shapefile (required)\n");

    return ;
}
