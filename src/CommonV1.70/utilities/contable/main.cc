/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the contable program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: January 6, 2010
   >>>> Modifications: June 22, 2010 - Added calculation of Kappa Statistic
   >>>>                August 17, 2010: Converted into a general utility program.
   >>>>                August 17, 2010: Allowed for use without GDAL.
   >>>>                July 30, 2012 - Modified for version 1.59 of Common Image software.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "contable.h"
#include "params/params.h"
#include <image/image.h>
#include <iostream>
#ifdef GDAL
#include <gdal_priv.h>
#endif
#include <cstring>
#include <climits>
#include <cstdlib>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");

extern Image testImage;
extern Image classifiedImage;

// Forward function declarations
void usage();
void help();

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for the contable program
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           opens log file (if requested), calls the contable function and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: January 6, 2010.
| Modifications: August 17, 2010: Converted into a general utility program.
|                August 17, 2010: Allowed for use without GDAL.
|
------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  int status = true;

#ifdef GDAL
  GDALAllRegister();
#endif

  if (argc == 1)
  {
    usage();
    cout << "ERROR: Need parameter file as argument." << endl;
    return EXIT_FAILURE;
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: contable -h or contable -help" << endl << endl;
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

// Call contable function
    status = contable();
  }

  testImage.close();
  classifiedImage.close();

  if (status)
  {
    cout << endl << "Successful completion of contable program" << endl;
  }
  else
  {
    cout << endl << "The contable program terminated improperly." << endl;
  }
  
  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
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
|       Written: January 6, 2010.
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "contable parameter_file_name" << endl << endl;
  cout << "For help information: contable -h or contable -help" << endl;
  cout << "For version information: contable -v or contable -version" << endl;

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
|       Written: January 6, 2010.
| Modifications: August 17, 2010: Allowed for use without GDAL.
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The contable progam is called in the following manner:" << endl;
  cout << endl << "contable_setup parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: contable -h or contable -help" << endl;
  cout << endl << "For version information: contable -v or contable -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input Files:\n"
"-test_image		(string)	Input test set image data (required)\n"
#ifdef GDAL
"-classified_image	(string)	Input classified image data (required)\n");
#else
"-classified_image	(string)	Input classified image data (required)\n"
"-ncols			(int)		Number of columns in input images\n"
"				        (0 < ncols < %d, required)\n"
"-nrows			(int)		Number of rows in input images\n"
"				        (0 < nrows < %d, required)\n"
"-dtype			(string)	Input image data type.\n"
"					Currently must be either:\n"
"					 UInt8   => \"unsigned char (8-bit)\" or\n"
"					 UInt16  => \"unsigned short int\n"
"						     (16-bit)\" or\n"
"					 Float32 => \"float (32-bit)\"\n"
"					(required)\n"
"NOTE: The Test image and Classified iamge must have the same number of columns and rows\n"
"      and data type\n",USHRT_MAX,USHRT_MAX);
#endif

  cout << endl << "Contable result is printed to standard output." << endl;
  return;
}
