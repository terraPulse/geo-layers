/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for hseg_extract/hsegextract - Hierarchical Segmentation Feature Extract - program
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: July 6, 2009 (Based on earlier versions of rhseg_read/hsegreader and feature_extract programs)
   >>>> Modifications: July 8, 2009: Final clean-up of code for release
   >>>>                July 24, 2009: Minor bug fix for hseg_out_nregions and hseg_out_thresholds
   >>>>                October 6, 2009: Added robustness to the reading of parameter values from parameter files
   >>>>                December 5, 2010 - Reorganized code so that program will not fail when run remotely in parameter file mode
   >>>>                                   even when compiled with GTKMM
   >>>>                May 10, 2011 - Added capability to convert information from selected hierarchical level to a shapefile 
   >>>>                               (raster to vector conversion)
   >>>>                September 12, 2011 - Finished corrections to raster to vector conversion code.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <defines.h>
#include "hsegextract.h"
#include "params/initialParams.h"
#include <params/params.h>
#ifdef GTKMM
#include "params/paramsGUI.h"
#endif
#include <image/image.h>
#include <iostream>
#include <cstring>
#ifdef GDAL
#include <gdal_priv.h>
#endif
#ifdef GTKMM
#include <gtkmm.h>
#endif

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

/*-----------------------------------------------------------
|
|  Routine Name: main - Main program for Hierarchical Segmentation
|
|  Purpose: Declares program parameters, reads parameter file and initializes program parameters,
|           calls the hsegextract function and returns an exit status.
|
|  Input from command line (no input from command line for GUI version) - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: July 6, 2009 (Based on earlier versions of rhseg_read/hsegreader and feature_extract programs)
| Modifications: July 8, 2009: Final clean-up of code for release
|
------------------------------------------------------------*/
int main(int argc, char **argv)
{
  bool status = true;

#ifdef GDAL
  GDALAllRegister();
#endif

  params.gtkmm_flag = false;

  if (argc == 1)
  {
#ifdef GTKMM
    Gtk::Main kit(argc, argv);
    params.gtkmm_flag = true;
    ParamsGUI paramsGUI;
    Gtk::Main::run(paramsGUI);
    status = params.status;
#else
    usage();
    cout << "ERROR: Need parameter file as argument." << endl;
    return EXIT_FAILURE;
#endif
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: hsegextract -h or hsegextract -help" << endl << endl;
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
      status = initialParams.read(argv[1]);
#ifdef GTKMM
      if (!status)
      {
        Gtk::Main kit(argc, argv);
        params.gtkmm_flag = true;
        ParamsGUI paramsGUI;
        if (initialParams.oparam_flag)
          paramsGUI.set_oparam_file(initialParams.oparam_file);
        Gtk::Main::run(paramsGUI);
        status = params.status;
      }
#endif
    }
  }

  if (status)
  {
    initialParams.print();
    status = hsegextract();
  }
  else
  {
    usage();
    cout << "ERROR: Error reading parameter file (read)" << endl;
    return EXIT_FAILURE;
  }

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
#ifdef GTKMM
    cout << "hsegextract" << endl;
    cout << "or" << endl;
#endif
    cout << "hsegextract parameter_file_name" << endl << endl;
    cout << "For help information: hsegextract -h or hsegextract -help" << endl;
    cout << "For version information: hsegextract -v or hsegextract -version" << endl;

    return ;
}

void help()
{
#ifdef GTKMM
    cout << endl << "The hsegextract progam can be called in two different manners:" << endl;
    cout << endl << "hsegextract" << endl;
    cout << endl << "(where the program parameters are entered via a Graphical User Interface)" << endl;
    cout << endl << "or" << endl;
    cout << endl << "hsegextract parameter_file_name" << endl;
    cout << endl << "(where the program parameters are read from a parameter file)" << endl;
    cout << endl << "In the later case, \"parameter_file_name\" is the name of the input parameter" << endl;
#else
    cout << endl << "The hsegextract progam is called in the following manner:" << endl;
    cout << endl << "hsegextract parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
#endif
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: hsegextract -h or hsegextract -help" << endl;
    cout << "For version information: hsegextract -v or hsegextract -version" << endl;
#ifdef GTKMM
    cout << endl << "The (optional) parameter file consists of entries of the form:" << endl;
#else
    cout << endl << "The parameter file consists of entries of the form:" << endl;
#endif
    cout << endl << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
    fprintf(stdout,"-oparam	        	    (string)	HSeg/RHSeg output parameter file\n"
"					(required)\n"
"-hseg_level	(short unsigned int)	Hierarchical segmentation level at which\n"
"					the selected feature values are extracted.\n"
"					(required)\n\n");

    fprintf(stdout,"One or more of the following are required:\n\n"
"-class_labels_map_ext	    (string)	File name for the class labels map\n"
"					extracted at the selected\n"
"					hierarchical segmentation level.\n"
"-class_npix_map_ext	    (string)	File name for the class # of pixels map\n"
"					extracted at the selected\n"
"					hierarchical segmentation level.\n"
"-class_mean_map_ext	    (string)	File name for the class mean feature map\n"
"					extracted at the selected\n"
"					hierarchical segmentation level.\n"
"-class_std_dev_map_ext	    (string)	File name for the class standard\n"
"					deviation feature map extracted at the\n"
"					selected hierarchical segmentation level.\n"
"-class_bpratio_map_ext	    (string)	File name for the class boundary pixel\n"
"					ratio feature map extracted at the\n"
"					selected hierarchical segmentation level.\n"
#ifdef SHAPEFILE
"-class_shapefile_ext       (string)	Base file name for the class shapefile\n"
"					to contain the segmentation information\n"
"					extracted at the selected hierarchical\n"
"					segmentation level.\n"
#endif
"-object_labels_map_ext	    (string)	File name for the object labels map\n"
"					extracted at the selected\n"
"					hierarchical segmentation level.\n"
"-object_npix_map_ext	    (string)	File name for the object # of pixels map\n"
"					extracted at the selected\n"
"					hierarchical segmentation level.\n"
"-object_mean_map_ext	    (string)	File name for the object mean feature map\n"
"					extracted at the selected\n"
"					hierarchical segmentation level.\n"
"-object_std_dev_map_ext     (string)	File name for the object standard\n"
"					deviation feature map extracted at the\n"
"					selected hierarchical segmentation level.\n"
#ifdef SHAPEFILE
"-object_bpratio_map_ext     (string)	File name for the object boundary pixel\n"
"					ratio feature map extracted at the\n"
"					selected hierarchical segmentation level.\n"
"-object_shapefile_ext       (string)	Base file name for the object shapefile\n"
"					to contain the segmentation information\n"
"					extracted at the selected hierarchical\n"
"					segmentation level.\n");
#else
"-object_bpratio_map_ext     (string)	File name for the object boundary pixel\n"
"					ratio feature map extracted at the\n"
"					selected hierarchical segmentation level.\n");
#endif

fprintf(stdout,"\nNOTE: The \"object\" files will be ignored if the\n"
"\"object_labels_map\" is not specified in the HSeg/RHSeg output parameter file\n"
"(-oparam above).\n\n");
#ifdef SHAPEFILE
fprintf(stdout,"\nNOTE: If the \"object_labels_map\" is specified, a region class\n"
"shapefile cannot be produced without an accompanying region object shapefile.\n\n");
#endif
    return ;
}
