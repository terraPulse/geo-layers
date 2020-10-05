/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the make_multiband program - with GDAL dependencies
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: February 16, 2014
   >>>> Modifications: February 21, 2014 - Upgraded to properly handle projection and geotransform information.
   >>>>                February 22, 2014 - Improved IO efficiency by writing with a direct call to GDAL's RasterIO function.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "params/params.h"
#include <image/image.h>
#include <gdal_priv.h>
#include <iostream>
#include <cstdlib>
#include <climits>
#include <cstring>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");

extern Image inputImage;

// Forward function declarations
void usage();
void help();

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for the make_multiband program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           opens log file (if requested), creates the multiband iamge and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: February 16, 2014.
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
    cout << "For help information: make_multiband -h or make_multiband -help" << endl << endl;
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

  Image outputImage;

  if (status)
  {
// Print program parameters
    params.print();

// Create ouputImage with the same structure as the inputImage (first band) - but with the desired number of bands
    status = outputImage.create(params.output_image_file, inputImage, params.nbands);
  }

  if (status)
  {
 // Copy the metadata (if any)
    char** metadata = inputImage.get_metadata("");
    if (metadata != NULL)
      outputImage.set_metadata(metadata,"");
 // Copy the multiband data to the outputImage
    int band, ncols, nrows;
    GDALDataset  *inputDataset, *outputDataset;
    outputDataset = outputImage.get_imageDataset();
    string input_image_file;
    ncols = inputImage.get_ncols();
    nrows = inputImage.get_nrows();
    float *buffer = new float[ncols*nrows];
    for (band = 0; band < params.nbands; band++)
    {
      cout << "Processing band " << (band + 1) << endl;
      if (band != 0)
      {
        input_image_file = params.input_base + params.suffix_list[band];
        inputImage.open(input_image_file);
      }
      inputDataset = inputImage.get_imageDataset();
      GDALRasterBand *rb = inputDataset->GetRasterBand(1);
      GDALRasterBand *wb = outputDataset->GetRasterBand(band+1);
      rb->RasterIO(GF_Read, 0, 0, ncols, nrows, buffer, ncols, nrows, GDT_Float32, 0, 0);
      wb->RasterIO(GF_Write, 0, 0, ncols, nrows, buffer, ncols, nrows, GDT_Float32, 0, 0);
      inputImage.close();
    }
  }
  outputImage.print_info();
  outputImage.close();

  if (status)
  {
    cout << endl << "Successful completion of make_multiband program" << endl;
  }
  else
  {
    cout << endl << "The make_multiband program terminated improperly." << endl;
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
|       Written: February 16, 2014.
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "make_multiband parameter_file_name" << endl << endl;
  cout << "For help information: make_multiband -h or make_multiband -help" << endl;
  cout << "For version information: make_multiband -v or make_multiband -version" << endl;

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
|       Written: February 16, 2014.
| Modifications: 
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The make_multiband progam is called in the following manner:" << endl;
  cout << endl << "make_multiband parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: make_multiband -h or make_multiband -help" << endl;
  cout << endl << "For version information: make_multiband -v or make_multiband -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input file specification:\n"
"-input_base		(string)	Base of multiple input images (required)\n"
"-suffix_list		(string)	The set of suffixes of the multiband\n"
"					image to be combined (required,\n"
"					 a comma delimited list)\n");
  fprintf(stdout,"Output File:\n"
"-output_image		(string)	Output image (required)\n");

  return;
}
