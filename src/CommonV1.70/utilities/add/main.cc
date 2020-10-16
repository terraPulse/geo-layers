/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the add program - with optional GDAL dependencies
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: July 7, 2010
   >>>> Modifications: July 19, 2012 - Added add_value
   >>>>                July 19, 2012 - Modified to work without GDAL
   >>>>                July 19, 2012 - Modified for version 1.59 of Common Image software.
   >>>>	               Aug. 21, 2012 - Added optional copy_color_table flag.
   >>>>                Jan. 8, 2014 - Change type of add_value from int to float
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "params/params.h"
#include <image/image.h>
#ifdef GDAL
#include <gdal_priv.h>
#endif
#include <iostream>
#include <cstdlib>
#include <climits>
#include <cstring>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");

extern Image inputImage1;
extern Image inputImage2;

// Forward function declarations
void usage();
void help();

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for the add program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           opens log file (if requested), calls the add function and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: July 7, 2010.
| Modifications: 
|
------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  bool status = true;

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
    cout << "For help information: add -h or add -help" << endl << endl;
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

// Create ouputImage as a copy of inputImage1
#ifdef GDAL
    if (!outputImage.create_copy(params.output_image_file, inputImage1))
      status = outputImage.create(params.output_image_file, inputImage1);
#else
    status = outputImage.create(params.output_image_file, inputImage1);
#endif
  }

  if (status)
  {
// Call add function
    if (params.input_image2_flag)
      status = outputImage.math(inputImage1,inputImage2,Add);
    else
      status = outputImage.math(inputImage1,params.add_value,Add);
  }

#ifdef GDAL
  if (status)
  {
// Copy color table from input image if present and requested
    if (inputImage1.get_imageDataset()->GetRasterBand(1)->GetColorTable() != NULL)
    {
      if (params.copy_color_table_flag)
        outputImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(inputImage1.get_imageDataset()->GetRasterBand(1)->GetColorTable());
      else
        outputImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(NULL);
    }
  }
#endif

  inputImage1.close();
  if (params.input_image2_flag)
    inputImage2.close();
  outputImage.close();

  if (status)
  {
    cout << endl << "Successful completion of add program" << endl;
  }
  else
  {
    cout << endl << "The add program terminated improperly." << endl;
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
|       Written: July 7, 2010.
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "add parameter_file_name" << endl << endl;
  cout << "For help information: add -h or add -help" << endl;
  cout << "For version information: add -v or add -version" << endl;

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
|       Written: July 7, 2010.
| Modifications: July 19, 2012 - Added add_value
|                July 19, 2012 - Modified to work without GDAL
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The add progam is called in the following manner:" << endl;
  cout << endl << "add parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: add -h or add -help" << endl;
  cout << endl << "For version information: add -v or add -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input Files or parameters:\n"
#ifdef GDAL
"-input_image1		(string)	First input image (required)\n"
"One of the following two must be specified:\n"
"-input_image2		(string)	Second input image\n"
#else
"-input_image1		(string)	First input image (required)\n"
"-ncols			(int)		Number of columns in first input image\n"
"				        (0 < ncols < %d, required)\n"
"-nrows			(int)		Number of rows in first input image\n"
"				        (0 < nrows < %d, required)\n"
"-nbands		(int)		Number of spectral bands in first input image\n"
"					(0 < nbands < %d, required)\n"
"-dtype			(string)	Input image data type. Must be either:\n"
"					 UInt8   => \"unsigned char (8-bit)\" or\n"
"					 UInt16  => \"unsigned short int\n"
"						     (16-bit)\" or\n"
"					 Float32 => \"float (32-bit)\"\n"
"					(required)\n",USHRT_MAX,USHRT_MAX,USHRT_MAX);
  fprintf(stdout,"One of the following two must be specified:\n"
"-input_image2		(string)	Second input image. Must be same\n"
"					dimensions as first input image\n"
#endif // !GDAL
"-add_value		(float)		Value to add to input_image1\n"
"(If second input image is specified, then add_value will be ignored\n");
  fprintf(stdout,"Output File and associated parameter:\n"
"-output_image		(string)	Output image (required)\n"
"-copy_color_table	(bool)		Flag to request copying of the color\n"
"					table from the first input image to the\n"
"					output image, if present in the first \n"
"					input image (true (1) or false (0),\n"
"					 default = false)\n");

  cout << endl << "The add program adds input_image2 or add_value to input_image1 producing output_image" << endl;

  return;
}
