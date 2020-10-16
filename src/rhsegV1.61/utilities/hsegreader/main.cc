/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for hseg_read/hsegreader - Hierarchical Segmentation Reader - program
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
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <defines.h>
#include "hsegreader.h"
#include "params/initialParams.h"
#include <params/params.h>
#ifdef GTKMM
#include "params/paramsGUI.h"
#endif
#include <iostream>
#ifdef GTKMM
#include <gtkmm.h>
#endif

using namespace std;
using namespace HSEGTilton;
#ifdef GTKMM
using namespace CommonTilton;
#endif

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
|           opens log file (if requested), calls the hseg function and returns an exit status.
|
|  Input from command line (no input from command line for GUI version) - parameter_file_name
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: November 15, 2007.
| Modifications: May 12, 2009 - Modified user interface and data I/O for compatibility with ILIADS project needs.
|                July 8, 2009: Final clean-up of code for release
|
------------------------------------------------------------*/
int main(int argc, char **argv)
{
  bool status = true;

#ifdef GTKMM
  Gtk::Main kit(argc, argv);
  params.gtkmm_flag = true;
  ParamsGUI paramsGUI;
#endif

  if (argc == 1)
  {
#ifdef GTKMM
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
    cout << "For help information: hsegreader -h or hsegreader -help" << endl << endl;
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
      if (status)
        paramsGUI.hide();
      else
      {
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
    if (initialParams.debug > 0)
    {
      params.log_fs.open(initialParams.log_file.c_str(),ios_base::out);
      status = params.log_fs.is_open();
      if (!status)
      {
        usage();
        cout << "ERROR: Error opening log file: " << initialParams.log_file << endl;
        return EXIT_FAILURE;
      }
    }
  }
  else
  {
    usage();
    cout << "ERROR: Error reading parameter file (read)" << endl;
    return EXIT_FAILURE;
  }

  if (status)
  {
    initialParams.print();
#ifdef GTKMM
    RegionClass::set_static_vals();
    RegionObject::set_static_vals(initialParams.region_class_nghbrs_list_flag,
                                  initialParams.region_object_nghbrs_list_flag);
    HSEGReader hsegReader;
    Gtk::Main::run(hsegReader);
#else
    status = hsegreader();
#endif
  }

// Close log file
  if (params.log_fs.is_open())
     params.log_fs.close();

  if (status)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

void usage() // Informs user of proper usage of program when mis-used.
{
    cout << endl << "Usage: " << endl << endl;
#ifdef GTKMM
    cout << "hsegreader" << endl;
    cout << "or" << endl;
#endif
    cout << "hsegreader parameter_file_name" << endl << endl;
    cout << "For help information: hsegreader -h or hsegreader -help" << endl;
    cout << "For version information: hsegreader -v or hsegreader -version" << endl;

    return ;
}

void help()
{
#ifdef GTKMM
    cout << endl << "The hsegreader progam can be called in two different manners:" << endl;
    cout << endl << "hsegreader" << endl;
    cout << endl << "(where the program parameters are entered via a Graphical User Interface)" << endl;
    cout << endl << "or" << endl;
    cout << endl << "hsegreader parameter_file_name" << endl;
    cout << endl << "(where the program parameters are read from a parameter file)" << endl;
    cout << endl << "In the later case, \"parameter_file_name\" is the name of the input parameter" << endl;
#else
    cout << endl << "The hsegreader progam is called in the following manner:" << endl;
    cout << endl << "hsegreader parameter_file_name" << endl;
    cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
#endif
    cout << "file. For contents see below." << endl;
    cout << endl << "For this help: hsegreader -h or hsegreader -help" << endl;
    cout << "For version information: hsegreader -v or hsegreader -version" << endl;
#ifdef GTKMM
    cout << endl << "The (optional) parameter file consists of entries of the form:" << endl;
#else
    cout << endl << "The parameter file consists of entries of the form:" << endl;
#endif
    cout << endl << "-parameter_name parameter_value(s)" << endl;

    fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
    fprintf(stdout,"-oparam			 (string)	HSeg/RHSeg output parameter file\n"
"					(required)\n"
"-region_class_nghbrs_list  (bool)	Flag to request the inclusion of the list\n"
"					of the region classes neighboring each\n"
"					region object.\n"
"					(true (1) or false (0), default = false)\n"
"-region_object_nghbrs_list (bool)	Flag to request the inclusion of the list\n"
"					of the region objects neighboring each\n"
"					region object.\n"
"					(true (1) or false (0), default = false)\n"
"-debug	     (short unsigned int)	Debug option\n"
"					(0 < debug < 255, default = 1)\n");

    fprintf(stdout,"\nThe following parameter must be specified if debug > 0 (ignored if debug = 0):\n\n"
"-log			 (string)	Output log file (default = \"hsegreader.log\")\n");

    return ;
}


