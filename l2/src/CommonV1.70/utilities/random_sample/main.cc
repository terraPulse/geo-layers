/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the random_sample program - with GDAL dependencies
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
   >>>> Modifications: July 31, 2012 - Modified for version 1.59 of Common Image software.
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

extern Image inputImage;

// Forward function declarations
void usage();
void help();

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for the random_sample program interface
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           opens log file (if requested), calls the random_sample function and returns an exit status.
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
    cout << "For help information: random_sample -h or random_sample -help" << endl << endl;
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

// Create ouputImage as geo-referenced copy of inputImage
    if (params.color_table_flag)
    {
      outputImage.create(params.output_image_file, inputImage, inputImage.get_nbands(), 
                         inputImage.get_data_type(), "BMP");
    }
    else
    {
      if (!outputImage.create_copy(params.output_image_file, inputImage))
        outputImage.create(params.output_image_file, inputImage);
    }

// Call random_sample function
    status = outputImage.random_sample(inputImage,params.rate);

// Set color table
    if (inputImage.get_imageDataset()->GetRasterBand(1)->GetColorTable() != NULL)
      outputImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(inputImage.get_imageDataset()->GetRasterBand(1)->GetColorTable());
    else if (params.color_table_flag)
    {
      GDALColorTable colorTable(GPI_RGB);
      GDALColorEntry *colorEntry;
      colorEntry = new GDALColorEntry;
   
      fstream color_table_fs;
      color_table_fs.open(params.color_table_file.c_str(),ios_base::in);

      int entry_index, entry_value;
      int sub_pos;
      string line,sub_string;

      for (entry_index = 0; entry_index < 256; entry_index++)
      {
        getline(color_table_fs,line);
        sub_pos = line.find(":");
        sub_string = line.substr(0,sub_pos);
        entry_value = atoi(sub_string.c_str());
        if (entry_value != entry_index)
        {
          cout << "ERROR: Color Table has unexpected format" << endl;
          return false;
        }
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c1 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c2 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c3 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c4 = atoi(sub_string.c_str());
        colorTable.SetColorEntry(entry_value,colorEntry);
      }
      color_table_fs.close();

      outputImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(&colorTable);
    }
  }

  inputImage.close();
  outputImage.close();

  if (status)
  {
    cout << endl << "Successful completion of random_sample program" << endl;
  }
  else
  {
    cout << endl << "The random_sample program terminated improperly." << endl;
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
  cout << "random_sample parameter_file_name" << endl << endl;
  cout << "For help information: random_sample -h or random_sample -help" << endl;
  cout << "For version information: random_sample -v or random_sample -version" << endl;

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
| Modifications: 
|
------------------------------------------------------------*/
void help()
{
  cout << endl << "The random_sample progam is called in the following manner:" << endl;
  cout << endl << "random_sample parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: random_sample -h or random_sample -help" << endl;
  cout << endl << "For version information: random_sample -v or random_sample -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThe following parameters must be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input File:\n"
"-input_image		(string)	Input image (required)\n"
"-rate			(float)		Percentage rate of pixels to be selected\n"
"					from the input image for output to the\n"
"					output image (required)\n"
"-color_table		(string)	256 entry RGB Color Table (file) for output\n"
"					classified image (as listed by gdalinfo,\n"
"				        optional)\n");
  fprintf(stdout,"Output File:\n"
"-output_image		(string)	Output image (required)\n");

  cout << endl << "The random_sample program randomly selects a specified percentage of pixels from the input_image" << endl;
  cout << "for output to the output_image" << endl;

  return;
}
