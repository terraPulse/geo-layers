/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the utm_convert program - requires GDAL
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: September 12, 2012
   >>>> Modifications: 
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "params/params.h"
#include "utm_convert.h"
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
|  Routine Name: main - Main program for the utm_convert program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           calls the utm_convert function and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: September 12, 2012.
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
    cout << "For help information: utm_convert -h or utm_convert -help" << endl << endl;
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

// Call utm_convert function
    if (params.lat_long_flag)
      status = lat_long_to_utm(params.latitude,params.longitude,params.WGS_84_UTM_Zone,params.UTM_X,params.UTM_Y);
    else
      status = utm_to_lat_long(params.UTM_X,params.UTM_Y,params.WGS_84_UTM_Zone,params.latitude,params.longitude);
  }

  if (status)
  {
    if (params.lat_long_flag)
    {
      cout << "(Latitude,Longitude) = (" << params.latitude << "," << params.longitude << ") corresponds to " << endl;
      cout << "(UTM_X,UTM_Y) = (" << params.UTM_X << "," << params.UTM_Y << ") for WGS UTM Zone " << params.WGS_84_UTM_Zone << endl;
    }
    else
    {
      cout << "(UTM_X,UTM_Y) = (" << params.UTM_X << "," << params.UTM_Y << ") corresponds to " << endl;
      cout << "(Latitude,Longitude) = (" << params.latitude << "," << params.longitude << ") for WGS UTM Zone " << params.WGS_84_UTM_Zone << endl;
    }
    cout << endl << "Successful completion of utm_convert program" << endl;
    return EXIT_SUCCESS;
  }
  else
  {
    cout << endl << "The utm_convert program terminated improperly." << endl;
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
|       Written: September 12, 2012.
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "utm_convert parameter_file_name" << endl << endl;
  cout << "For help information: utm_convert -h or utm_convert -help" << endl;
  cout << "For version information: utm_convert -v or utm_convert -version" << endl;

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
|       Written: September 12, 2012.
| Modifications: 
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The utm_convert progam is called in the following manner:" << endl;
  cout << endl << "utm_convert parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: utm_convert -h or utm_convert -help" << endl;
  cout << endl << "For version information: utm_convert -v or utm_convert -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input parameters:\n"
"-latitude		(double)	Latitude in degrees\n"
"-longitude		(double)	Longitude in degrees\n"
"\nOR\n\n"
"-UTM_X			(double)	UTM X coordinate in meters\n"
"-UTM_Y			(double)	UTM Y coordinate in meters\n");
  fprintf(stdout,"The following must be specified:\n"
"-WGS_84_UTM_Zone	(int)		WGS 84 / UTM Zone Number (required)\n");

cout << "If the latitude and longitude values are provided, utm_convert outputs the corresponding UTM_X and UTM_Y values" << endl;
cout << "If the UTM_X and UTM_Y values are provided, utm_convert outputs the corresponding latitude and longitude values" << endl;

  return;
}
