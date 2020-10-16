/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the hsegpvote program
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
   >>>>       Written: January 14, 2010
   >>>> Modifications: January 17, 2010 - Added 4nn connected component labeling.
   >>>>                August 10, 2010 - Added Kappa statistic and average and individual class accuracies
   >>>>                August 11, 2010 - Added output of class optimized classification
   >>>>                August 18, 2010 - Converted into an RHSeg utility
   >>>>                March 2, 2011 - Added color_table option (instead of obtaining color table from
   >>>>                                the pixel-based classification)
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <defines.h>
#include "hsegpvote.h"
#include "params/initialParams.h"
#include <params/params.h>
#include <image/image.h>
#include <gdal_priv.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace CommonTilton;
using namespace HSEGTilton;

// Globals
InitialParams initialParams;
Params params("Version 1.61, August 18, 2014");
oParams oparams;

// Externals
extern Image pixelClassImage;
extern Image trainingLabelImage;

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
    cout << "For help information: hsegpvote -h or hsegpvote -help" << endl << endl;
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
      if (status)
      {
        params.log_fs.open(initialParams.log_file.c_str(),ios_base::out);
        status = params.log_fs.is_open();
      }
      else
      {
        usage();
        cout << "ERROR: Error reading parameter file (initialParams.read)" << endl;
        return EXIT_FAILURE;
      }
    }
  }

  if (status)
  {
// Call hsegpvote function
    status = hsegpvote();
  }

  pixelClassImage.close();
  trainingLabelImage.close();
  initialParams.remove_temp_files();
  params.log_fs.close();

  if (status)
  {
    cout << endl << "Successful completion of hsegpvote program" << endl;
  }
  else
  {
    cout << endl << "The hsegpvote program terminated improperly." << endl;
  }
  
  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "hsegpvote parameter_file_name" << endl << endl;
    cout << "For help information: hsegpvote -h or hsegpvote -help" << endl;
    cout << "For version information: hsegpvote -v or hsegpvote -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The hsegpvote progam is called in the following manner:" << endl;
    cout << "hsegpvote parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter file" << endl;
    cout << "For contents see below." << endl;
    cout << endl << "For this help: hsegpvote -h or hsegpvote -help" << endl;
    cout << "For version information: hsegpvote -v or hsegpvote -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;
    cout << endl << "Each parameter name and its value(s) must be entered on a separate, single line" << endl;

    fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n");
    fprintf(stdout,"\nInput Files:\n"
"-pixel_classification	(string)	Pixel-based classification (required)\n"
"-oparam				(string)	HSeg/RHSeg output parameter file\n"
"						(required)\n"
"-training_labeling	(string)	Training label data (required)\n");
    fprintf(stdout,"\nOther input parameters:\n"
"-region_class_flag	(boolean)	If true, use region classes even when\n"
"						region objects are available\n"
"						Can only be true for 4nn connectivity\n"
"						(true (1) or false (0), default = false)\n"
"-color_table		(string)	256 entry RGB Color Table (file) for output\n"
"						classified image (as listed by gdalinfo,\n"
"						optional)\n");

    fprintf(stdout,"\nOutput Files:\n"
"-opt_classification	(string)	Class optimized classification (required)\n"
"						Will be in same format as input\n"
"						pixel_classification file.\n"
"-log_file		(string)	Log file (required)\n");

    return ;
}
