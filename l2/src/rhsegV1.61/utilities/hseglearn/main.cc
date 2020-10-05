/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the hseg_learn/HSegLearn - Hierarchical Segmentation Learn by Example - program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: December 21, 2011
   >>>> Modifications: January 4, 2012 - Modified to make output files (label_out and ascii_out) compatible with hsegviewer
   >>>>                March 30, 2012 - Modified GUI to add a highlight regions step in the circle region of interest scenario
   >>>>                April 19, 2012 - Added a crosshair that tracks between all displayImages 
   >>>>                April 19, 2012 - The circle drawn for the circle ROI options now shows in all displayImages
   >>>>                April 19, 2012 - Added a button through which the HSWO/HSeg/RHSeg log file can be displayed.
   >>>>                May 15, 2012 - Modified initialization of Images with respect to geotransform information
   >>>>                May 23, 2012 - Added capability to handle Images with different spatial resolutions
#ifdef GLSIMP
   >>>>		       June 22, 2012 - Modified the parameter input GUI allow input of the "-input_image" if this
   >>>>				       parameter is not provided in the *.oparam file.
#endif
   >>>>                August 1, 2012 - Modified for version 1.59 of Common Image software.
   >>>>                August 20, 2012 - Added capability to reset red, green, blue display bands without restarting
   >>>>                                  the program.
   >>>>                September 11, 2012 - Modified label_out image output to include geotransform and projection information,
   >>>>                                     if available.
   >>>>                November 6, 2012 - Added ability to turn on and turn off mode to highlight regions by mouse clicking.
   >>>>                November 12, 2012 - Added "Undo" button to undo the last positive/negative examples submit.
   >>>>		       December 26, 2012 - Fixed bug in hseglearn - related to new undo option
   >>>>		       February 7, 2013 - Fixed bug in hseglearn (error check for .erase when item not found in list)
   >>>>		       February 12, 2013 - Changed "Clear all Highlighted Regions" to "Undo Last Region(s) Highlight"
   >>>>		       March 4, 2013 - Added back "Clear all Highlighted Regions" (in addition to "Undo last Region Highlight(s)")
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <defines.h>
#include "hseglearn.h"
#include "params/initialParams.h"
#include <params/params.h>
#include "params/paramsGUI.h"
#include <image/image.h>
#include <iostream>
#include <fstream>
#include <gdal_priv.h>
#include <gtkmm.h>

using namespace std;
using namespace HSEGTilton;
using namespace CommonTilton;

//Globals
InitialParams initialParams;
#ifdef THREEDIM
Params params("Version 1.61 (3-D), August 18, 2014");
#else
Params params("Version 1.61 (2-D), August 18, 2014");
#endif
oParams oparams;

// Forward function declarations
void usage();
void help();

int main(int argc, char *argv[])
{
  bool status;

  GDALAllRegister();

  Gtk::Main kit(argc, argv);
  params.gtkmm_flag = true;
  ParamsGUI paramsGUI;

  if (argc == 1)
  {
    Gtk::Main::run(paramsGUI);
    status = params.status;
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: hseglearn -h or hseglearn -help" << endl << endl;
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
      return false;
    }
    else
    {
      status = initialParams.read(argv[1]);
      if (status)
        paramsGUI.hide();
      else
      {
        if (initialParams.oparam_flag)
          paramsGUI.set_oparam_file(initialParams.oparam_file);
        Gtk::Main::run(paramsGUI);
        status = params.status;
      }
    }
  }

  if (status)
  {
//    initialParams.print();
//    params.print();
    RegionClass::set_static_vals();
    RegionObject::set_static_vals(false,false);
    HSegLearn hsegLearn;
    Gtk::Main::run(hsegLearn);
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "hseglearn" << endl;
    cout << "or" << endl;
    cout << "hseglearn parameter_file_name" << endl << endl;
    cout << "For help information: hseglearn -h or hseglearn -help" << endl;
    cout << "For version information: hseglearn -v or hseglearn -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The hseglearn progam can be called in two different manners:" << endl;
    cout << endl << "hseglearn (where the program parameters are entered via a" << endl;
    cout << "            Graphical User Interface)" << endl;
    cout << "or" << endl;
    cout << "hseglearn parameter_file_name (where the program parameters are read from a" << endl;
    cout << "                                parameter file)" << endl;
    cout << endl << "In the later case, \"parameter_file_name\" is the name of the input parameter file" << endl;
    cout << "(STRING) (for contents see below)." << endl;
    cout << endl << "For this help: hseglearn -h or hseglearn -help" << endl;
    cout << "For version information: hseglearn -v or hseglearn -version" << endl;
    cout << endl << "The (optional) parameter file consists of entries of the form:" << endl << endl;
    cout << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
#ifdef GLSIMP
    fprintf(stdout,"-input_image		(string)	Input image data file name (If\n"
"					 -input_image is NOT provided in the\n"
"					 -oparam file, it MUST be provided here\n"
"					 BEFORE -oparam. However, this value will\n"
"					 be overridden by the value provided in \n"
"					 the -oparam file if also provided there)\n"
"-oparam                 (string)	HSeg/RHSeg output parameter file\n"
"					(required)\n"
#else
    fprintf(stdout,"-oparam                 (string)	HSeg/RHSeg output parameter file\n"
"					(required)\n"
#endif
"-red_display_band	(short)		Spectral band from the input image data\n"
"					selected for display as red (required)\n"
"-green_display_band	(short)		Spectral band from the input image data\n"
"					selected for display as green (required)\n"
"-blue_display_band	(short)		Spectral band from the input image data\n"
"					selected for display as blue (required)\n"
"-RGB_image_stretch	(int)		RGB Image stretch option\n"
"					  1.\"Linear Stretch with Percent Clipping\",\n"
"					  2.\"Histogram Equalization\",\n"
"					  3.\"Linear Stretch to Percentile Range\"\n"
"					(default: 2. \"Histogram Equalization\")\n"
"-range			(2*float)	Percent Clipping or Percentile Range\n"
"					for RGB Image stretch.\n"
"					Provide two floating point numbers\n"
"					specifying minimum and maximum range\n"
"					(default for minimum: 0.1, 0.0 to 1.0 allowed,\n"
"					 default for maximum: 0.9, 0.0 to 1.0 allowed,\n"
"					 Ignored for RGB_image_stretch = 2)\n"
"-examples_out	        (string)	Output examples list (ASCII)\n"
"					(default = examples_out.txt)\n"
"-label_out	        (string)	Output label map file\n"
"					(default = label_out.tif)\n"
"-ascii_out	        (string)	Output ASCII class label names list\n"
"					(default = ascii_out.txt)\n"
#ifdef THREEDIM
"-view_dimension	        (string)	Dimension from which a selected element\n"
"					is displayed in 2-D display\n"
"					(default = slice, allowed values =\n"
"					 column, row or slice)\n"
"-view_element		(short)		Selected element for 2-D display\n"
"					(default = 0)\n"
#endif
"-examples_in		(string)	Input examples list (ASCII)\n"
"					(no default)\n"
"-panchromatic_image	(string)	Input Panchromatic Image (no default)\n"
"-reference_image	(string)	Input Reference Image (no default)\n");

    return ;
}
