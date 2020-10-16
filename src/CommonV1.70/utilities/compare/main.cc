/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the compare program - with GDAL dependencies
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: June 13, 2011
   >>>> Modifications: March 9, 2012 - Changed "Compare" to "Equal" and added "LessThan" and "MoreThan" cases.
   >>>>                July 19, 2012 - Modified for version 1.59 of Common Image software.
   >>>>	               Aug. 21, 2012 - Added optional copy_color_table flag.
   >>>>                Oct. 22, 2012 - Added output_value parameter.
   >>>>                Jan.  8, 2014 - Changed type of compare_value from int to float
   >>>>                Feb. 22, 2014 - Added the proper setting of the "no data value"
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "params/params.h"
#include <image/image.h>
#include <gdal_priv.h>
#include <iostream>

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
|  Routine Name: main - Main program for the compare program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           opens log file (if requested), calls the compare function and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: June 13, 2011.
| Modifications: 
|
------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  int status = true;

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
    cout << "For help information: compare -h or compare -help" << endl << endl;
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
    if (!outputImage.create_copy(params.output_image_file, inputImage1))
      status = outputImage.create(params.output_image_file, inputImage1);
  }

  if (status)
  {
// Call compare function
    if (params.input_image2_flag)
      status = outputImage.math(inputImage1,inputImage2,params.compare_type,params.output_value);
    else if (params.compare_value_flag)
      status = outputImage.math(inputImage1,params.compare_value,params.compare_type,params.output_value);
  }

  if (status)
  {
// Set and write out no_data_value
    outputImage.set_no_data_value(0);
    outputImage.put_no_data_value();
// Copy color table from input image if present and requested
    if (inputImage1.get_imageDataset()->GetRasterBand(1)->GetColorTable() != NULL)
    {
      if (params.copy_color_table_flag)
        outputImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(inputImage1.get_imageDataset()->GetRasterBand(1)->GetColorTable());
      else
        outputImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(NULL);
    }
  }

  inputImage1.close();
  if (params.input_image2_flag)
    inputImage2.close();
  outputImage.close();

  if (status)
  {
    cout << endl << "Successful completion of compare program" << endl;
  }
  else
  {
    cout << endl << "The compare program terminated improperly." << endl;
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
|       Written: June 13, 2011.
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "compare parameter_file_name" << endl << endl;
  cout << "For help information: compare -h or compare -help" << endl;
  cout << "For version information: compare -v or compare -version" << endl;

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
|       Written: June 13, 2011.
| Modifications: 
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The compare progam is called in the following manner:" << endl;
  cout << endl << "compare parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: compare -h or compare -help" << endl;
  cout << endl << "For version information: compare -v or compare -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input Files and related parameters:\n"
"-input_image1		(string)	First input image (required)\n"
"-compare_type		(string)	Comparison Type - Valid values are:\n"
"					  Equal,\n"
"					  LessThan, or \n"
"					  MoreThan.\n"
"					 (default = Equal)\n"
"One of the following two must be specified:\n"
"-input_image2		(string)	Second input image\n"
"-compare_value		(float)		Value to compare input_image1 to\n"
"(If second input image is specified, then the compare_value will be ignored\n");
  fprintf(stdout,"Output File and related parameters:\n"
"-output_image		(string)	Output image (required)\n"
"-output_value		(int)		Output value for true comparison\n"
"					locations (default = 1,\n"
"					 valid 1 through 255)\n"
"-copy_color_table	(bool)		Flag to request copying of the color\n"
"					table from the first input image to the\n"
"					output image, if present in the first \n"
"					input image (true (1) or false (0),\n"
"					 default = false)\n");

  cout << endl << "The compare program compares input_image1 to input_image2 or value producing output_image" << endl;

  return;
}
