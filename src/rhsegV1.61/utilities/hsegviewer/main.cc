/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the hseg_view/HSegViewer - Hierarchical Segmentation Viewer - program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: November 15, 2007
   >>>> Modifications: May 12, 2009: Modified user interface and data I/O for compatibility with ILIADS project needs.
   >>>>                July 8, 2009: Final clean-up of code for release
   >>>>                July 24, 2009: Minor bug fix for hseg_out_nregions and hseg_out_thresholds
   >>>>                October 6, 2009: Added robustness to the reading of parameter values from parameter files
   >>>>                May 15, 2012 - Modified initialization of Images with respect to geotransform information
   >>>>                May 15, 2012 - Corrected problems in displayimage class with respect to displayimage resizing.
#ifdef GLSIMP
   >>>>		       June 22, 2012 - Modified the parameter input GUI allow input of the "-input_image" if this
   >>>>				       parameter is not provided in the *.oparam file.
#endif
   >>>>                August 1, 2012 - Modified for version 1.59 of Common Image software.
   >>>>                August 20, 2012 - Added capability to reset red, green, blue display bands without restarting
   >>>>                                  the program.
   >>>>                September 11, 2012 - Modified label_out image output to include geotransform and projection information,
   >>>>                                     if available.
   >>>>                July 17, 2013 - Corrected code so as to enable hsegviewer_3d to function properly and hsegviewer to function
   >>>>                                properly when GDAL was not used in producing the RHSeg/HSeg output.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <defines.h>
#include "hsegviewer.h"
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
    cout << "For help information: hsegviewer -h or hsegviewer -help" << endl << endl;
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
    HSegViewer hsegViewer;
    Gtk::Main::run(hsegViewer);
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
    cout << "hsegviewer" << endl;
    cout << "or" << endl;
    cout << "hsegviewer parameter_file_name" << endl << endl;
    cout << "For help information: hsegviewer -h or hsegviewer -help" << endl;
    cout << "For version information: hsegviewer -v or hsegviewer -version" << endl;

    return ;
}

void help()
{
    cout << endl << "The hsegviewer progam can be called in two different manners:" << endl;
    cout << endl << "hsegviewer (where the program parameters are entered via a" << endl;
    cout << "            Graphical User Interface)" << endl;
    cout << "or" << endl;
    cout << "hsegviewer parameter_file_name (where the program parameters are read from a" << endl;
    cout << "                                parameter file)" << endl;
    cout << endl << "In the later case, \"parameter_file_name\" is the name of the input parameter file" << endl;
    cout << "(STRING) (for contents see below)." << endl;
    cout << endl << "For this help: hsegviewer -h or hsegviewer -help" << endl;
    cout << "For version information: hsegviewer -v or hsegviewer -version" << endl;
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
"					  1.\"No Stretch\",\n"
"					  2.\"Histogram Equalization\",\n"
"					  3.\"Linear Stretch to Percentile Range\"\n"
"					(default: 2. \"Histogram Equalization\")\n"
"-percentile_range	(2*float)	Percentile Range for RGB Image Stretch.\n"
"					Provide two floating point numbers\n"
"					specifying minimum and maximum range\n"
"					(default for minimum: 0.1, 0.0 to 1.0 allowed,\n"
"					 default for maximum: 0.9, 0.0 to 1.0 allowed,\n"
"					 used only for RGB_image_stretch = 3)\n"
"-grey_scale		(bool)		Region Class Grey Scale Display Flag\n"
"					Value 1 selects grey scale display for\n"
"					Region Class display. Value 0 selects\n"
"					pseudo color display. (default = 0)\n"
"-label_out	        (string)	Output class label map\n"
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
"-label_in		(string)	Input class label map (no default)\n"
"-ascii_in		(string)	Input ASCII class label names list\n"
"					(no default)\n"
"-reference1		(string)	Input Reference File (1) (no default)\n"
"-reference2		(string)	Input Reference File (2) (no default)\n");

    return ;
}
