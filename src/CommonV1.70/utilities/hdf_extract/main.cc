/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the hdf_extract program - requires GDAL
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: February 15, 2014
   >>>> Modifications: February 21, 2014 - Upgraded to properly handle projection and geotransform information.
   >>>>                February 22, 2014 - Improved IO efficiency by writing with a direct call to GDAL's RasterIO function.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "hdf_extract.h"
#include "params/params.h"
#include <gdal_priv.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");

// Forward function declarations
void usage();
void help();

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for the hdf_extract program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           calls the hdf_extract function and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: February 15, 2014.
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
    cout << "For help information: hdf_extract -h or hdf_extract -help" << endl << endl;
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

// Call hdf_extract function
    status = hdf_extract();
  }

  if (status)
  {
    cout << endl << "Successful completion of hdf_extract program" << endl;
    return EXIT_SUCCESS;
  }
  else
  {
    cout << endl << "The hdf_extract program terminated improperly." << endl;
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
|       Written: February 15, 2014.
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "hdf_extract parameter_file_name" << endl << endl;
  cout << "For help information: hdf_extract -h or hdf_extract -help" << endl;
  cout << "For version information: hdf_extract -v or hdf_extract -version" << endl;

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
|       Written: February 15, 2014.
| Modifications: 
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The hdf_extract progam is called in the following manner:" << endl;
  cout << endl << "hdf_extract parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: hdf_extract -h or hdf_extract -help" << endl;
  cout << endl << "For version information: hdf_extract -v or hdf_extract -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters must be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input parameters:\n"
"-input_HDF		(string)	Input HDF file name\n"
"-subdatasets		(string)	The set of subdatasets to be extracted\n"
"					(a comma delimited list -\n"
"					 valid subdataset names can\n"
"					 be found with gdal_info)\n");
  fprintf(stdout,"\nOutput parameter:\n"
"-output_image		(string)	Output image file name\n\n");

  fprintf(stdout,"\nThe following parameter may also be specified:\n"
"-output_format		(string)	Output image format (optional,\n"
"					 default = GTiff (for GeoTIFF), must\n"
"					 be a format recognized by GDAL)\n");

  return;
}
