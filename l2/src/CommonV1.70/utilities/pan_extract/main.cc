/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the pan_extract program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: June 19, 2012 (Based on register and a portion of hseglearn)
   >>>> Modifications: July 30, 2012 - Modified for version 1.59 of Common Image software.
   >>>>
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "params/params.h"
#include <image/image.h>
#include "pan_extract.h"
#include <iostream>

using namespace std;
using namespace CommonTilton;

//Globals
Params params("Version 1.70, February 22, 2014");

extern Image multispectralImage;
extern Image panchromaticImage;

// Forward function declarations
void usage();
void help();

int main(int argc, char *argv[])
{
  bool status = true;

  GDALAllRegister();

  if (argc == 1)
  {
    usage();
    status = false;
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: pan_extract -h or pan_extract -help" << endl << endl;
    status = false;
  }
  else if ((strncmp(argv[1],"-h",2) == 0) || (strncmp(argv[1],"-help",5) == 0))
  {
    help();
    status = false;
  }
  else if (strncmp(argv[1],"-",1) == 0)
  {
    usage();
    cout << "The parameter file name cannot start with an \"-\"" << endl;
    status = false;
  }
  else
  {
    if (argc != 2)
    {
      usage();
      status = false;
    }
    else
    {
      status = false;
      status = params.read(argv[1]);
    }
  }

  if (status)
  {  
    params.print();
    status = pan_extract();
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "pan_extract parameter_file_name" << endl << endl;
    cout << "For help information: pan_extract -h or pan_extract -help" << endl;
    cout << "For version information: pan_extract -v or pan_extract -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The pan_extract progam is called as follows:" << endl;
    cout << endl << "pan_extract parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: pan_extract -h or pan_extract -help" << endl;
    cout << endl << "For version information: pan_extract -v or pan_extract -version" << endl;
    cout << endl << "The parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specifed in the input parameter file:\n\n");
    fprintf(stdout,"-multispectral_image	(string)	Input multispectral image (required)\n"
"						Must include georeferencing information\n"
"-panchromatic_image		(string)	Input panchromatic image (required)\n"
"						Must include georeferencing information\n"
"-extracted_panchromatic_image	(string)	Output extracted panchromatic image (required)\n");

    cout << endl << "The output extracted_panchromatic_image will have the same format as the" << endl;
    cout << "input panchromatic image, but will cover the same geographic area as the" << endl;
    cout << "multispectral image." << endl;

    return ;
}
