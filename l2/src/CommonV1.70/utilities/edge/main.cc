/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the edge program - with optional GDAL dependencies
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: April 15, 2013
   >>>> Modifications: April 22, 2013 - Added edge_threshold parameter and changed edge function call interface.
   >>>>                May   22, 2013 - Added scale_output and output_mask_image parameters
   >>>>                May   28, 2013 - Replaced band_average flag with output_type parameter
   >>>>                July  26, 2013 - Added "Std_Dev" operation type and optional input mask 
   >>>>                Sept. 25, 2013 - Added the Frei-Chen_option parameter for edge, line and edge-line options of Frei-Chen operator
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

extern Image inputImage;
extern Image maskImage;

// Forward function declarations
void usage();
void help();

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for the edge program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           opens log file (if requested), calls the select edge function (Prewitt, Sobel, Scharr or Frei-Chen) 
|           and returns an exit status.
|
|  Input from command line - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: April 15, 2013
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
    cout << "For help information: edge -h or edge -help" << endl << endl;
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
  Image outputMaskImage;

  if (status)
  {
// Print program parameters
    params.print();

#ifdef GDAL
    if (params.output_type == 1)
      status = outputImage.create(params.output_image_file, inputImage, inputImage.get_nbands(), GDT_Float32);
    else
      status = outputImage.create(params.output_image_file, inputImage, 1, GDT_Float32);
    status = outputMaskImage.create(params.output_mask_image_file, inputImage, 1, GDT_Byte);
#else
    if (params.output_type == 1)
      status = outputImage.create(params.output_image_file, inputImage, inputImage.get_nbands(), Float32);
    else
      status = outputImage.create(params.output_image_file, inputImage, 1, Float32);
    status = outputMaskImage.create(params.output_mask_image_file, inputImage, 1, UInt8);
#endif
  }

  if (status)
  {
   if (params.mask_flag)
     maskImage.set_no_data_value(params.mask_value);
// Call selected edge function
    outputImage.edge(inputImage, maskImage, params.bias_value, params.edge_operation, params.edge_threshold, params.output_type, outputMaskImage, params.fc_option);
    if (params.edge_operation == Std_Dev)
    {
     // Want local minimum in 3x3 window for this case
      Image copyOutputImage, copyMaskImage;
      string temp_output_file_name, temp_mask_file_name;
      temp_output_file_name = params.temp_file_name + "output";
      temp_mask_file_name = params.temp_file_name + "mask";
#ifdef GDAL
      if (params.output_type == 1)
        status = copyOutputImage.create(temp_output_file_name, inputImage, inputImage.get_nbands(), GDT_Float32);
      else
        status = copyOutputImage.create(temp_output_file_name, inputImage, 1, GDT_Float32);
      status = copyMaskImage.create(temp_mask_file_name, inputImage, 1, GDT_Byte);
#else
      if (params.output_type == 1)
        status = copyOutputImage.create(temp_output_file_name, inputImage, inputImage.get_nbands(), Float32);
      else
        status = copyOutputImage.create(temp_output_file_name, inputImage, 1, Float32);
      status = copyMaskImage.create(temp_mask_file_name, inputImage, 1, UInt8);
#endif
      bool data_valid_flag;
      int col, row, band;
      int ncols, nrows, nbands;
      int nb_col, nb_row;
      double min_value, max_value;
      ncols = outputImage.get_ncols();
      nrows = outputImage.get_nrows();
      nbands = outputImage.get_nbands();
      max_value = 0;
      for (band = 0; band < nbands; band++)
      {
        min_value = outputImage.getMaximum(band,outputMaskImage);
        if (min_value > max_value)
          max_value = min_value;
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
            copyOutputImage.put_data(outputImage.get_data(col,row,band),col,row,band);
      }
      copyOutputImage.flush_data();
      copyOutputImage.computeMinMax();
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
          copyMaskImage.put_data(outputMaskImage.get_data(col,row,0),col,row,0);
      copyMaskImage.flush_data();
      copyMaskImage.set_no_data_value(0.0);
      copyMaskImage.computeMinMax();
      for (band = 0; band < nbands; band++)
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            min_value = max_value;
            if (copyMaskImage.data_valid(col,row,0))
              min_value = copyOutputImage.get_data(col,row,band);
            data_valid_flag = false;
            for (nb_row = (row-1); nb_row <= (row+1); nb_row++)
              for (nb_col = (col-1); nb_col <= (col+1); nb_col++)
              {
                if (copyMaskImage.data_valid(nb_col,nb_row,0))
                {
                  data_valid_flag = true;
                  if (copyOutputImage.get_data(nb_col,nb_row,band) < min_value)
                    min_value = copyOutputImage.get_data(nb_col,nb_row,band);
                }
              }
            if (data_valid_flag)
            {
              outputImage.put_data(min_value,col,row,band);
              if (band == 0)
                outputMaskImage.put_data(1.0,col,row,0);
            }
          }
      outputImage.flush_data();
      outputMaskImage.flush_data();

      copyOutputImage.close();
      copyMaskImage.close();
      remove(temp_output_file_name.c_str());
      remove(temp_mask_file_name.c_str());
      temp_output_file_name += ".hdr";
      temp_mask_file_name += ".hdr";
      remove(temp_output_file_name.c_str());
      remove(temp_mask_file_name.c_str());
    }
  }

  if ((status) && (params.scale_output_flag))
  {
// Scale the result, if requested
    outputImage.scale_offset(0.0, 1.0, outputMaskImage);
  }
  outputImage.computeMinMax();
  outputImage.print_info();

  inputImage.close();
  if (params.mask_flag)
    maskImage.close();
  outputImage.close();
  outputMaskImage.close();

  if (status)
  {
    cout << endl << "Successful completion of edge program" << endl;
  }
  else
  {
    cout << endl << "The edge program terminated improperly." << endl;
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
|       Written: April 15, 2013
| Modifications: 
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
  cout << "edge parameter_file_name" << endl << endl;
  cout << "For help information: edge -h or edge -help" << endl;
  cout << "For version information: edge -v or edge -version" << endl;

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
|       Written: April 15, 2013
| Modifications: April 22, 2013 - Added edge_threshold parameter
|                May   22, 2013 - Added scale_output and output_mask_image parameters
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The edge progam is called in the following manner:" << endl;
  cout << endl << "edge parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: edge -h or edge -help" << endl;
  cout << endl << "For version information: edge -v or edge -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
#ifdef GDAL
  fprintf(stdout,"Input File and parameter:\n"
"-input_image		(string)	Input image (required)\n");
#else
  fprintf(stdout,"Input File and parameters:\n"
"-input_image		(string)	Input image (required)\n"
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
#endif // !GDAL
  fprintf(stdout,"-mask			(string)	Input data mask file name\n"
"					Data type must be unsigned char.\n"
"					(default = {none})\n"
"-mask_value		(int)		If input data mask file is provided,\n"
"					this is the value in the mask file that\n"
#ifdef GDAL
"					designates bad data. (default is\n"
"					 \'NoData Value\' from image header,\n"
"					 if provided - otherwise 0.)\n"
#else
"					designates bad data. (default = 0.)\n"
#endif
"-bias_value		(int)		Bias value to add to the input data\n"
"					values before the edge value is\n"
"					computed. (Optional, default = 0.)\n"
"-edge_operation		(string)	Edge operation type (required)\n"
"					Must be either:\n"
"					  Prewitt\n"
"					  Sobel\n"
"					  Scharr\n"
"					  Frei-Chen or\n"
"					  Std_Dev\n"
"-Frei-Chen_option		(int)	Frei-Chen operation option:\n"
"					  1: edge\n"
"					  2: line-edge\n"
"					(default, 1: edge. Valid only for\n"
"					 the Frei-Chen edge_operation.)\n"
"-edge_threshold		(float)		Edge threshold value (default = 0.0)\n"
"-output_type		(int)		Output Type:\n"
"					  1. \"Multispectral,\"\n"
"					  2. \"Band Average,\"\n"
"					  3. \"Band Maximum,\"\n"
"					  4. \"Band Minimum,\"\n"
"					(default = 1 -> \"Multispectral\")\n"
"-scale_output		(bool)		Flag to request scaling of output edge\n"
"					values to std. dev. = 1.0 and min. = 0.0.\n"
"					(true (1) or false (0), default = false)\n");
  fprintf(stdout,"Output File:\n"
"-output_image		(string)	Output image (required)\n"
"-output_mask_image	(string)	Output mask image (required)\n");

  return;
}
