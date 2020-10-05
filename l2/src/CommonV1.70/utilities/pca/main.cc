/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the pca program - requires OpenCV and GDAL
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: April 7, 2014
   >>>> Modifications: 
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "pca.h"
#include "params/params.h"
#include <gdal_priv.h>
#include <iostream>
#include <cstdlib>
#include <climits>
#include <cstring>

using namespace std;

//Globals
Params params("Version 1.70, April 7, 2014");

// Forward function declarations
void usage();
void help();

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for the pca program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           calls the pca function and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: April 4, 2014.
| Modifications: 
|
------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  bool status = true;

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
    cout << "For help information: pca -h or pca -help" << endl << endl;
    return EXIT_SUCCESS;
  }
  else if ((strncmp(argv[1],"-h",2) == 0) || (strncmp(argv[1],"-help",5) == 0))
  {
    help();
    return EXIT_SUCCESS;
  }
  else if (strncmp(argv[1],"-",1) == 0)
  {
    usage();
    cout << "ERROR: The parameter file name cannot start with an \"-\"" << endl;
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
      status = params.read(argv[1]);
      if (!status)
      {
        usage();
        cout << "ERROR: Error reading parameter file (read)" << endl;
        return EXIT_FAILURE;
      }
    }
  }

  if (status)
  {
// Print program parameters
    params.print();

// Call pca function
    status = pca();
  }

  if (status)
  {
    cout << endl << "Successful completion of pca program" << endl;
    return EXIT_SUCCESS;
  }
  else
  {
    cout << endl << "The pca program terminated improperly." << endl;
    return EXIT_FAILURE;
  }
  
}

/*-----------------------------------------------------------
|
|  Routine Name: usage - Usage function
|
|       Purpose: Informs user of proper usage of program when mis-used.
|
|         Input:
|
|        Output:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: April 4, 2014.
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "pca parameter_file_name" << endl << endl;
  cout << "For help information: pca -h or pca -help" << endl;
  cout << "For version information: pca -v or pca -version" << endl;

  return;
}

/*-----------------------------------------------------------
|
|  Routine Name: help - Help function
|
|       Purpose: Provides help information to user on program parameters
|
|         Input:
|
|        Output:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: April 4, 2014.
| Modifications: 
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The pca progam is called in the following manner:" << endl;
  cout << endl << "pca parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: pca -h or pca -help" << endl;
  cout << endl << "For version information: pca -v or pca -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters must be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input Files or parameters:\n"
"-input_image		(string)	Input image file name (required)\n"
"-number_components	(integer)	Number of components to be retained\n"
"					(optional, no default)\n"
"-variance		(float)		Retained percent variance (optional,\n"
"					 default = 95%%, ignored if\n"
"					 number_components specified)\n");
  fprintf(stdout,"Output File:\n"
"-output_image		(string)	Output image (required)\n");

  return;
}
