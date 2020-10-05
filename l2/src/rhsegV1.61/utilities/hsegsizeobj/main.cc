/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the hsegsizeobj program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>    Written By: James C. Tilton, NASA GSFC, Mail Code 606.3, Greenbelt, MD 20771
   >>>>                James.C.Tilton@nasa.gov
   >>>>
   >>>>       Written: September 10, 2014
   >>>> Modifications:
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <defines.h>
#include "hsegsizeobj.h"
#include "params/initialParams.h"
#include "params/params.h"
#include <gdal_priv.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace HSEGTilton;

// Globals
InitialParams initialParams;
Params params("Version 1.61, September 10, 2014");
oParams oparams;

// Forward function declarations
void usage();
void help();

int main(int argc, char *argv[])
{
  bool status;

  GDALAllRegister();

  if (argc == 1)
  {
    usage();
    cout << "ERROR: Need parameter file as argument." << endl;
    return EXIT_FAILURE;
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: hsegsizeobj -h or hsegsizeobj -help" << endl << endl;
    return EXIT_FAILURE;
  }
  else if ((strncmp(argv[1],"-h",2) == 0) || (strncmp(argv[1],"-help",5) == 0))
  {
    help();
    return EXIT_FAILURE;
  }
  else if (strncmp(argv[1],"-",1) == 0)
  {
    usage();
    cout << "The parameter file name cannot start with an \"-\"" << endl;
    return EXIT_FAILURE;
  }
  else
  {
    if (argc != 2)
    {
      usage();
      cout << "ERROR: Incorrect number of parameters on command line" << endl;
      return EXIT_FAILURE;
    }
    else
    {
      status = initialParams.read(argv[1]);
      if (!status)
      {
        usage();
        cout << "ERROR: Error reading parameter file (initialParams.read)" << endl;
        return EXIT_FAILURE;
      }
    }
  }

  if (status)
  {
// Call hsegsizeobj function
    status = hsegsizeobj();
  }

  if (status)
  {
    cout << endl << "Successful completion of hsegsizeobj program" << endl;
    return EXIT_SUCCESS;
  }
  else
  {
    cout << endl << "The hsegsizeobj program terminated improperly." << endl;
    return EXIT_FAILURE;
  }

}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "hsegsizeobj parameter_file_name" << endl << endl;
    cout << "For help information: hsegsizeobj -h or hsegsizeobj -help" << endl;
    cout << "For version information: hsegsizeobj -v or hsegsizeobj -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The hsegsizeobj progam is called in the following manner:" << endl;
    cout << "hsegsizeobj parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter file" << endl;
    cout << "For contents see below." << endl;
    cout << endl << "For this help: hsegsizeobj -h or hsegsizeobj -help" << endl;
    cout << "For version information: hsegsizeobj -v or hsegsizeobj -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;
    cout << endl << "Each parameter name and its value(s) must be entered on a separate, single line" << endl;

    fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n");
    fprintf(stdout,"\nInput File:\n"
"-oparam			(string)	HSeg/RHSeg output parameter file\n"
"					(required)\n");

    fprintf(stdout,"\nOutput Files:\n"
"-sizenpix		(string)	Number of pixels image at\n"
"					selected hierarchical levels (required)\n"
"-sizeobj		(string)	Region object label image at\n"
"					selected hierarchical levels (optional)\n"
"-sizethres   (string)  A list of comma seperated size thresholds (required)\n"
"NOTE: The output image files will have the same format as the \"object_labels_map\" specified in \"oparam\"\n");

    return ;
}
