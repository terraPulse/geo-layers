/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the rhseg program - with gtkmm and GDAL dependencies
   >>>>
   >>>>  Private:
   >>>> 	main(int argc, char **argv)
   >>>> 	usage()
   >>>> 	help()
   >>>>
   >>>>   Static:
   >>>>   Public:
   >>>>
   >>>>       Written: January 28, 2009
   >>>> Modifications: May 12, 2009: Modified user interface and data I/O for compatibility with ILIADS project needs.
   >>>>                July 8, 2009: Final clean-up of code for release
   >>>>                July 24, 2009: Minor bug fix for hseg_out_nregions and hseg_out_thresholds
   >>>>                September 24, 2009: Bug fix for floating point input in parallel version
   >>>>                October 6, 2009: Added robustness to the reading of parameter values from parameter files
   >>>>                December 2, 2009 - Replaced use of strftime (with its time_buffer) with ctime.
   >>>>		       January 10, 2010: Changed the definition of min_npixels parameter
   >>>>		       January 19, 2010: Changed the definition of spclust_start parameter and renamed it spclust_max
   >>>>                May 26, 2010: Default redefined for split_pixels_factor for program modes HSWO and HSEG.
   >>>>                June 7, 2010: Modified operation of the RHSEG program mode, eliminating the need for the max_min_npixels parameter.
   >>>>                September 9, 2010 - Added facility to enter serial key information from GUI.
   >>>>                September 10, 2010 - Reintroduced optional small region merge acceleration factor.
   >>>>                December 5, 2010 - Reorganized code so that program will not fail when run remotely in parameter file mode
   >>>>                                   even when compiled with GTKMM.
   >>>>	               December 28, 2010 - Added spclust_min parameter.
   >>>>                May 16, 2011 - Added option for 4nn region object finding for 8nn or more analysis for HSWO, HSEG and RHSEG.
   >>>>                February 24, 2012 - Upgraded boundary_map from Byte to Short Integer
   >>>>                July 16, 2012 - Added capability to output segmentation map results in same image format as the input image data.
   >>>>		       March 1, 2013 - Expanded the use of the standard deviation spatial feature.
   >>>>                May 22, 2013 - Added edge input image (edgeInImage) and associated code to optionally utilize edge information
   >>>>                               to initialize the region growing process and influence the region growing process
   >>>>                May 24, 2013 - Added random_init_seed parameter
   >>>>                September 11, 2013 - Refined processing window artifact elimination with edge information.
   >>>>                September 16, 2013 - Added edge_dissim_option parameter.
   >>>>                September 16, 2013 - Added seam_edge_threshold parameter, and replaced previous window artifact elimination approach with
   >>>>                                     edge-based approach - even when edge_wght = 0.0. This requires input of edge_image even when 
   >>>>                                     edge_wght = 0.0 in RHSEG mode (not required for HSWO or HSEG modes).
   >>>>                September 16, 2013 - Eliminated split_pixels_factor, seam_threshold_factor and region_threshold_factor parameters.
   >>>>                February 7, 2014 - Added sort_flag parameter.
   >>>>                February 28, 2014 - Added initial_merge_flag parameter.
   >>>>                March 19, 2014 - Replaced the initial_merge_flag parameter with the initial_merge_npix parameter.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <defines.h>
#include "hseg.h"
#include <params/params.h>
#ifdef SERIALKEY
#include <serialkey/serialkey.h>
#ifdef GTKMM
#include <serialkey/serialkeyGUI.h>
#endif
#endif
#ifdef GTKMM
#include <params/initialGUI.h>
#include <params/outputFileGUI.h>
#include <params/paramsGUI.h>
#include <params/finalGUI.h>
#include <status/statustracker.h>
#endif
#ifdef GDAL
#include <image/image.h>
#include <gdal_priv.h>
#endif // GDAL
#include <iostream>
#include <ctime>
#ifdef GTKMM
#include <gtkmm.h>
#include <pthread.h>
#endif

using namespace std;
using namespace HSEGTilton;
#ifdef GDAL
using namespace CommonTilton;
#endif

//Globals
#ifdef THREEDIM
Params params("Version 1.61 (3-D), August 18, 2014");
#else
Params params("Version 1.61 (2-D), August 18, 2014");
#endif
oParams oparams;

#ifdef GDAL
extern Image inputImage;
extern Image maskImage;
extern Image stdDevInImage;
extern Image stdDevMaskImage;
extern Image edgeInImage;
extern Image edgeMaskImage;
extern Image regionMapInImage;
Image classLabelsMapImage;
Image boundaryMapImage;
Image objectLabelsMapImage;
#endif

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
|       Written: December 16, 2002.
| Modifications: May 20, 2003 - Added option to initialize with Classical Region Growing.
|                May 30, 2003 - Added classic_pred flag.  If TRUE, the Classical Region Growing
|                               predicate is used.  If FALSE, the region-based discriminate of
|                               HSeg is used.
|                August 19, 2003 - Reorganized program and added user controlled byteswapping
|                October 18, 2003 - Added boundary_npix_file option
|                October 31, 2003 - Added subrlblmap_file and nb_subregions_file options
|                November 5, 2003 - Revised Params structure
|                November 24, 2003 - Changed "sub" to "conn" (for connected regions) and added
|                                    full set of outputs for connected regions
|                March 2, 2004 - Changed extmean_flag to extmean_wght
|                March 2, 2004 - Added option to output boundary_map
|                April 2, 2004 - Added mask_value parameter
|                June 3, 2004 - Modified BSMSE and BMMSE to take square root
|                June 9, 2004 - Removed extmean option (ineffective)
|                March 1, 2005 - Updated and rearranged help information
|                March 10, 2005 - Added checks to prevent overwriting input files with any program output
|                May 9, 2005 - Added code for integration with VisiQuest
|                May 25, 2005 - Added temporary file I/O for faster processing of large data sets
|                August 9, 2005 - Added "conv_criterion" and "gstd_dev_crit" parameters
|                August 9, 2005 - Changed "rthreshlist" to "rconv_critlist"
|                October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 2, 2006 - Added scale and offset input parameters (for input data)
|                October 31, 2007 - Removed "conv_criterion," "conv_factor," "gdissim_crit" and "gstd_dev_crit"
|                                   parameters, and added "gdissim_flag" parameter
|                October 31, 2007 - Changed "rconv_critlist" back to "rthreshlist"
|                October 31, 2007 - Removed VisiQuest related code
|                November 9, 2007 - Combined region feature output into region_classes file
|                May 9, 2008 - Reorganized source code, including adding the "Params" object class.
|                July 28, 2008 - Added copyright notice.
|                August 1, 2008 - Modified function of "region_std_dev" parameter.
|                August 8, 2008 - Added hseg_out_thresholds and hseg_out_nregions parameters.
|                September 10, 2008 - Added percent completion (status) tracking.
|                May 12, 2009 - Modified user interface and data I/O for compatibility with ILIADS project needs.
|
------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  int status = true;
  params.gtkmm_flag = false;
  time_t now;

#ifdef GTKMM
  Glib::ustring strTime = " ";
  Glib::ustring strMessage = " ";
#endif

#ifdef GDAL

#ifdef THREEDIM
#ifdef GTKMM
  if (argc == 1)
  {
    Gtk::Main kit(argc, argv);
    strMessage += "\nGDAL may not be used with three-dimensional processing.\nUse rhseg_run directly.\n";
    Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    end_dialog.run();
  }
  else
#endif // GTKMM
    cout << "ERROR: GDAL may not be used with three-dimensional processing.\n       Use rhseg_run directly." << endl;
  exit(EXIT_FAILURE);
#endif // #ifdef THREEDIM

  GDALAllRegister();
#else  // NOTE: Should always use GDAL!

#ifdef GTKMM
  if (argc == 1)
  {
    Gtk::Main kit(argc, argv);
    strMessage += "\nGDAL must be used in rhseg.\n";
    Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    end_dialog.run();
  }
  else
#endif // GTKMM
    cout << "ERROR: GDAL must be used in rhseg." << endl;
  exit(EXIT_FAILURE);
#endif // else branch of ifdef GDAL

  if (argc == 1)
  {
#ifdef GTKMM
    Gtk::Main kit(argc, argv);
    params.gtkmm_flag = true;

#ifdef SERIALKEY
    bool serial_key_flag;
    serial_key_flag = serialkey();
    while (!serial_key_flag)
    {
      SerialkeyGUI serialkeyGUI;
      Gtk::Main::run(serialkeyGUI);
      if (params.status == 2)
        exit(EXIT_FAILURE);
      serial_key_flag = serialkey();
      if (!serial_key_flag)
      {
        strMessage = "\nInvalid Serial Key Information Entered. Try Again.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
      }
    }
#endif

    InitialGUI initialGUI;
    Gtk::Main::run(initialGUI);
    status = params.status;
    if (status == 1)
    {
      OutputFileGUI outputFileGUI;
      Gtk::Main::run(outputFileGUI);
      status = params.status;
    }
    if (status == 1)
    {
      ParamsGUI paramsGUI;
      Gtk::Main::run(paramsGUI);
      status = params.status;
    }
    if (status == 3)
      exit(EXIT_SUCCESS);
    if (status == 2)
    {
      if (params.debug > 0)
      {
        params.log_fs.open(params.log_file.c_str(),ios_base::out);
        status = params.log_fs.is_open();
      }
      if (status)
      {
        now = time(NULL);
        strTime = ctime(&now);
        switch (params.program_mode)
        {
          case 1:  strMessage = "Started processing HSWO at:  " + strTime;
                   break;
          case 2:  strMessage = "Started processing HSeg at:  " + strTime;
                   break;
          case 3:  strMessage = "Started processing RHSeg at:  " + strTime;
                   break;
          default: strMessage = "Started processing RHSeg at:  " + strTime;
                   break;
        }
      // Calculate program parameters
        if (status)
          status = params.calc();

        if (status)
        {
      // Print program parameters
          if (params.debug > 0)
            params.print();

        // create classLabelsMapImage, boundaryMapImage and objectLabelsMapImage
          int nbands = 1;
          GDALDataType data_type = GDT_UInt32;
          classLabelsMapImage.create(params.class_labels_map_file,inputImage,nbands,data_type,params.output_driver_description);
          if (params.boundary_map_flag)
          {
            data_type = GDT_UInt16;
            boundaryMapImage.create(params.boundary_map_file,inputImage,nbands,data_type,params.output_driver_description);
          }
          if (params.object_labels_map_flag)
          {
            data_type = GDT_UInt32;
            objectLabelsMapImage.create(params.object_labels_map_file,inputImage,nbands,data_type,params.output_driver_description);
          }
        // Call hseg function to execute hseg algorithm
          pthread_t pthread;
          int return_code;
          return_code = pthread_create(&pthread, NULL, hseg_thread, NULL);
          if (return_code)
          {
            strMessage += "\nError returned from pthread_create()\n";
            Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);

            end_dialog.run();
            exit(EXIT_FAILURE);
          }
          StatusTracker statusTracker;
          Gtk::Main::run(statusTracker);

          inputImage.close();
          if (params.mask_flag)
            maskImage.close();
          if (params.std_dev_image_flag)
            stdDevInImage.close();
          if (params.std_dev_mask_flag)
            stdDevMaskImage.close();
          if (params.edge_image_flag)
            edgeInImage.close();
          if (params.edge_mask_flag)
            edgeMaskImage.close();
          if (params.region_map_in_flag)
            regionMapInImage.close();
          classLabelsMapImage.close();
          if (params.boundary_map_flag)
            boundaryMapImage.close();
          if (params.object_labels_map_flag)
            objectLabelsMapImage.close();

          status = params.gtkmm_flag;
          params.gtkmm_flag = true;
          now = time(NULL);
          strTime = ctime(&now);
          switch (params.program_mode)
          {
            case 1:  strMessage += "\nSuccessful completion of HSWO at:  " + strTime;
                     break;
            case 2:  strMessage += "\nSuccessful completion of HSeg at:  " + strTime;
                     break;
            case 3:  strMessage += "\nSuccessful completion of RHSeg at:  " + strTime;
                     break;
            default: strMessage += "\nSuccessful completion of RHSeg at:  " + strTime;
                     break;
          }
          FinalGUI finalGUI(strMessage);
          Gtk::Main::run(finalGUI);
          status = params.status;
        }
        else // if (status)
        {
          switch (params.program_mode)
          {
            case 1:  strMessage = "\nThe HSWO program terminated improperly\n";
                     break;
            case 2:  strMessage = "\nThe HSeg program terminated improperly\n";
                     break;
            case 3:  strMessage = "\nThe RHSeg program terminated improperly\n";
                     break;
            default: strMessage = "\nThe RHSeg program terminated improperly\n";
                     break;
          } 
          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
          end_dialog.run();
        } // else branch of if (status)
      } // if (status)
    } // if (status == 2)
#else // !GTKMM
    usage();
    cout << "ERROR: Need parameter file as argument." << endl;
    exit(EXIT_FAILURE);
#endif // !GTKMM
  } //  if (argc == 1)
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: rhseg -h or rhseg -help" << endl << endl;
    exit(EXIT_SUCCESS);
  }
  else if ((strncmp(argv[1],"-h",2) == 0) || (strncmp(argv[1],"-help",5) == 0))
  {
    help();
    exit(EXIT_SUCCESS);
  }
  else if (strncmp(argv[1],"-",1) == 0)
  {
    usage();
    cout << "ERROR: The parameter file name cannot start with an \"-\"" << endl;
    exit(EXIT_FAILURE);
  }
  else
  {
    if (argc != 2)
    {
      usage();
      cout << "ERROR: Incorrect number of parameters on command line" << endl;
      exit(EXIT_FAILURE);
    }
    else
    {
      status = params.read_init(argv[1]);
      if (status)
      {
        if (params.debug > 0)
        {
          params.log_fs.open(params.log_file.c_str(),ios_base::out);
          status = params.log_fs.is_open();
        }
      }
      else
      {
        usage();
        cout << "ERROR: Error reading parameter file (read_init)" << endl;
        exit(EXIT_FAILURE);
      }
      if (status)
        status = params.read(argv[1]);
      else
      {
        usage();
        cout << "ERROR: Error opening log file: " << params.log_file << endl;
        exit(EXIT_FAILURE);
      }
    }
  } //  if (argc == 1) - else branch

  if (!params.gtkmm_flag)
  {
#ifdef SERIALKEY
   // The Serialkey code restricts usage of the compiled code to a specified period of time.
   if (status)
   {
    if (serialkey())
      status = true;
    else
    {
      cout << "ERROR: Error indicated exiting serialkey function" << endl;
      exit(EXIT_FAILURE);
    }
   }
   else
   {
    cout << "ERROR: Error reading parameter file (read)" << endl;
    exit(EXIT_FAILURE);
   }
#endif // SERIALKEY

   if (status)
   {
    now = time(NULL);
    if (params.debug > 0)
    {
      switch (params.program_mode)
      {
        case 1:  params.log_fs << endl << "Started processing HSWO at : " << ctime(&now) << endl;
                 break;
        case 2:  params.log_fs << endl << "Started processing HSeg at : " << ctime(&now) << endl;
                 break;
        case 3:  params.log_fs << endl << "Started processing RHSeg at : " << ctime(&now) << endl;
                 break;
        default: params.log_fs << endl << "Started processing RHSeg at : " << ctime(&now) << endl;
                 break;
      }
#if (defined(WINDOWS) || defined(CYGWIN))
      params.log_fs << endl << "Copyright (C) 2006 United States Government as represented by the" << endl;
#else
      params.log_fs << endl << "Copyright \u00a9 2006 United States Government as represented by the" << endl;
#endif
      params.log_fs << "Administrator of the National Aeronautics and Space Administration." << endl;
      params.log_fs << "No copyright is claimed in the United States under Title 17, U.S. Code." << endl;
      params.log_fs << "All Other Rights Reserved." << endl;
    }
#if (defined(WINDOWS) || defined(CYGWIN))
    cout << endl << "Copyright (C) 2006 United States Government as represented by the" << endl;
#else
    cout << endl << "Copyright \u00a9 2006 United States Government as represented by the" << endl;
#endif
    cout << "Administrator of the National Aeronautics and Space Administration." << endl;
    cout << "No copyright is claimed in the United States under Title 17, U.S. Code." << endl;
    cout << "All Other Rights Reserved." << endl;
    switch (params.program_mode)
    {
      case 1:  cout << endl << "Started processing HSWO at:  " << ctime(&now) << endl;
               break;
      case 2:  cout << endl << "Started processing HSeg at:  " << ctime(&now) << endl;
               break;
      case 3:  cout << endl << "Started processing RHSeg at:  " << ctime(&now) << endl;
               break;
      default: cout << endl << "Started processing RHSeg at:  " << ctime(&now) << endl;
               break;
    }
   }
   else
   {
    cout << "ERROR: Error reading parameter file (read)" << endl;
    exit(EXIT_FAILURE);
   }

// Calculate program parameters
   if (status)
    status = params.calc();

   if (status)
   {
// Print program parameters
    if (params.debug > 0)
      params.print();

  // create classLabelsMapImage, boundaryMapImage and objectLabelsMapImage
    int nbands = 1;
    GDALDataType data_type = GDT_UInt32;
    classLabelsMapImage.create(params.class_labels_map_file,inputImage,nbands,data_type,params.output_driver_description);
    if (params.boundary_map_flag)
    {
      data_type = GDT_UInt16;
      boundaryMapImage.create(params.boundary_map_file,inputImage,nbands,data_type,params.output_driver_description);
    }
    if (params.object_labels_map_flag)
    {
      data_type = GDT_UInt32;
      objectLabelsMapImage.create(params.object_labels_map_file,inputImage,nbands,data_type,params.output_driver_description);
    }

// Call hseg function to execute hseg algorithm
    status = hseg();
   }

   inputImage.close();
   if (params.mask_flag)
     maskImage.close();
   if (params.std_dev_image_flag)
     stdDevInImage.close();
   if (params.std_dev_mask_flag)
     stdDevMaskImage.close();
   if (params.edge_image_flag)
     edgeInImage.close();
   if (params.edge_mask_flag)
     edgeMaskImage.close();
   if (params.region_map_in_flag)
     regionMapInImage.close();
   classLabelsMapImage.close();
   if (params.boundary_map_flag)
     boundaryMapImage.close();
   if (params.object_labels_map_flag)
     objectLabelsMapImage.close();

  } // if (!params.gtkmm_flag)

  if (status)
  {
    now = time(NULL);
    if (params.debug > 0)
    {
      switch (params.program_mode)
      {
        case 1:  params.log_fs << endl << "Successful completion of HSWO at:  " << ctime(&now) << endl;
                 break;
        case 2:  params.log_fs << endl << "Successful completion of HSeg at:  " << ctime(&now) << endl;
                 break;
        case 3:  params.log_fs << endl << "Successful completion of RHSeg at:  " << ctime(&now) << endl;
                 break;
        default: params.log_fs << endl << "Successful completion of RHSeg at:  " << ctime(&now) << endl;
                 break;
      }
    }
    if (!params.gtkmm_flag)
    {
      switch (params.program_mode)
      {
        case 1:  cout << endl << "Successful completion of HSWO at:  " << ctime(&now) << endl;
                 break;
        case 2:  cout << endl << "Successful completion of HSeg at:  " << ctime(&now) << endl;
                 break;
        case 3:  cout << endl << "Successful completion of RHSeg at:  " << ctime(&now) << endl;
                 break;
        default: cout << endl << "Successful completion of RHSeg at:  " << ctime(&now) << endl;
                 break;
      }
    }
  }
  else
  {
    if (params.debug > 0)
    {
      switch (params.program_mode)
      {
        case 1:  params.log_fs << endl << "The HSWO program terminated improperly." << endl;
                 break;
        case 2:  params.log_fs << endl << "The HSeg program terminated improperly." << endl;
                 break;
        case 3:  params.log_fs << endl << "The RHSeg program terminated improperly." << endl;
                 break;
        default: params.log_fs << endl << "The RHSeg program terminated improperly." << endl;
                 break;
      }
    }
    if (!params.gtkmm_flag)
    {
      switch (params.program_mode)
      {
        case 1:  cout << endl << "The HSWO program terminated improperly." << endl;
                 break;
        case 2:  cout << endl << "The HSeg program terminated improperly." << endl;
                 break;
        case 3:  cout << endl << "The RHSeg program terminated improperly." << endl;
                 break;
        default: cout << endl << "The RHSeg program terminated improperly." << endl;
                 break;
      }
    }
  }
  
  if (params.log_fs.is_open())
    params.log_fs.close( );

  if (status)
    exit(EXIT_SUCCESS);
  else
    exit(EXIT_FAILURE);
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
|       Written: December 16, 2002.
| Modifications: May 9, 2008 - Updated usage text.
|                May 30, 2008 - Updated usage text.
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
#ifdef GTKMM
  cout << "rhseg" << endl;
  cout << "or" << endl;
#endif
  cout << "rhseg parameter_file_name" << endl << endl;
  cout << "For help information: rhseg -h or rhseg -help" << endl;
  cout << "For version information: rhseg -v or rhseg -version" << endl;

#if (defined(WINDOWS) || defined(CYGWIN))
  cout << endl << "Copyright (C) 2006 United States Government as represented by the" << endl;
#else
  cout << endl << "Copyright \u00a9 2006 United States Government as represented by the" << endl;
#endif
  cout << "Administrator of the National Aeronautics and Space Administration." << endl;
  cout << "No copyright is claimed in the United States under Title 17, U.S. Code." << endl;
  cout << "All Other Rights Reserved." << endl << endl;

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
|       Written: December 16, 2002.
| Modifications: March 1, 2005 - Reorganized and updated help text.
|                November 9, 2007 - Updated help text.
|                May 9, 2008 - Updated help text.
|                May 30, 2008 - Updated help text.
|                May 12, 2009: Modified user interface and data I/O for compatibility with ILIADS project needs.
|
------------------------------------------------------------*/
void help()
{
#ifdef GTKMM
  cout << endl << "The rhseg progam can be called in two different manners:" << endl;
  cout << endl << "rhseg" << endl;
  cout << endl << "(where the program parameters are entered via a Graphical User Interface)" << endl;
  cout << endl << "or" << endl;
  cout << endl << "rhseg parameter_file_name" << endl;
  cout << endl << "(where the program parameters are read from a parameter file)" << endl;
  cout << endl << "In the later case, \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: rhseg -h or rhseg -help" << endl;
  cout << endl << "For version information: rhseg -v or rhseg -version";
  cout << endl;
  cout << endl << "The (optional) parameter file consists of entries of the form:" << endl;
#else
  cout << endl << "The rhseg progam is called in the following manner:" << endl;
  cout << endl << "rhseg parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: rhseg -h or rhseg -help" << endl;
  cout << endl << "For version information: rhseg -v or rhseg -version";
  cout << endl;
  cout << endl << "The parameter file consists of entries of the form:" << endl;
#endif
  cout << endl << "-parameter_name parameter_value(s)" << endl;

  fprintf(stdout,"\nThis must be the first entry in the parameter file:\n"
"-program_mode		(string)	Program Mode - Valid values are:\n"
"					  HSWO,\n"
"					  HSEG, or \n"
"					  RHSEG.\n"
"					 (default = RHSEG)\n");

  fprintf(stdout,"\nThe following parameters may be specified in the input parameter file:\n\n");
  fprintf(stdout,"Input Files:\n"
"-input_image		(string)	Input image data file name (required)\n"
"-mask			(string)	Input data mask file name (optional,\n"
"					 default = {none})\n"
"-std_dev_image		(string)	Input standard deviation image file\n"
"					(optional, default = {none})\n"
"-std_dev_mask		(string)	Input standard devation mask file\n"
"					(optional, default = {none})\n"
"-edge_image		(string)	Input edge image file (required\n"
"					 for RHSEG mode, optional otherwise,\n"
"					 default = {none})\n"
"-edge_mask		(string)	Input edge mask file (optional,\n"
"					 default = {none})\n"
"-region_map_in		(string)	Input region map file (optional,\n"
"					 default = {none})\n");

  fprintf(stdout,"\nOther Required Parameters:\n"
"-spclust_wght		(float)		Relative importance of spectral\n"
"					clustering versus region growing\n"
"					(Value ignored and parameter set to\n"
"					 0.0 for program mode HSWO, otherwise,\n"
"					 0.0 <= spclust_wght <= 1.0,\n"
"					 no default)\n"
"-dissim_crit		(int)		Dissimilarity Criterion\n"
"					  1. \"1-Norm,\"\n"
"					  2. \"2-Norm,\"\n"
"					  3. \"Infinity Norm,\"\n"
"					  4. \"Spectral Angle Mapper,\"\n"
"					  5. \"Spectral Information Divergence,\"\n"
#ifdef MSE_SQRT
"					  6. \"Square Root of Band Sum Mean\n"
"					     Squared Error,\"\n"
"					  7. \"Square Root of Band Maximum Mean\n"
"					     Squared Error,\"\n"
#else
"					  6. \"Band Sum Mean Squared Error,\"\n"
"					  7. \"Band Maximum Mean Squared Error,\"\n"
#endif
"					  8. \"Normalized Vector Distance,\"\n"
"					  9. \"Entropy,\"\n"
"					 10. \"SAR Speckle Noise Criterion,\"\n"
//"					 11. \"Feature range.\"\n"
#ifdef MSE_SQRT
"					 (default: 6. \"Square Root of Band\n"
"					  Sum Mean Squared Error\")\n"
#else
"					 (default: 6.\"Band Sum Mean Squared Error\")\n"
#endif
"-log			(string)	Output log file (no default)\n\n");

  fprintf(stdout,"\nThe following optional parameters specify the scaling of the input image data:\n\n");
  fprintf(stdout,"-scale			(double)	Comma delimited list of input data\n"
"					scale factors (specify one value per\n"
"					 band, default = 1.0 for each band)\n"
"-offset			(double)	Comma delimited list of input data\n"
"					offset factors (specify one value per\n"
"					 band, default = 0.0 for each band)\n\n");

  fprintf(stdout,"Output Files (with default names - see User's Manual):\n"
"-class_labels_map	(string)	Output class labels map file name\n"
"					(required)\n"
"-boundary_map		(string)	Output hierarchical boundary map file\n"
"					name (optional)\n"
"-region_classes		(string)	Output region classes file name\n"
"					(required)\n"
"-oparam			(string)	Output parameter file name (required,\n"
"					 default =\n"
"					 \"\'input_image_file_name\'.oparam\")\n\n");

   fprintf(stdout,"Other optional output flags and files (with default names) are:\n"
"-object_conn_type1	(bool)		If 1 (true), create object labels map\n"
"					with \"-conn_type\" = 1, irrespective of\n"
"					the value of \"-conn_type\" (below),\n"
"					(default = 0 (false))\n"
"-object_labels_map	(string)	Output object labels map file name\n"
"-region_objects		(string)	Output region objects file name\n"
"NOTE: Both object_labels_map and region_objects must be specified if either is\n"
"specified! These files names are ignored for program mode HSWO with\n"
"\"-conn_type\" = 1 or for program mode HSWO for any value of \"-conn_type\"\n"
"when \"-object_conn_type1\" = false\n");

  fprintf(stdout,"\nThe following parameters select the optional contents of the required output\n"
"region_classes file and the optional output region_objects file (above):\n\n");
  fprintf(stdout,"-region_sum		(bool)		Flag to request the inclusion of the\n"
"					region sum feature values (also sumsq\n"
"					and sumxlogx, if available). \n"
"					(1 (true) or 1 (false),\n"
"					 default = 1 if nbands < 20,\n"
"					 default = 0 otherwise)\n"
"-region_std_dev		(bool)		Flag to request the inclusion of the\n"
"					region standard deviation feature\n"
"					values. (1 (true) or 0 (false),\n"
"					 default = 0.)\n"
"-region_boundary_npix	(bool)		Flag to request the inclusion of the\n"
"					region boundary number of pixels values\n"
"					(1 (true) or 0 (false),default = 0)\n"
"-region_threshold	(bool)		Flag to request the inclusion of the\n"
"					merge threshold for the most recent\n"
"					merge for each region.\n"
"					(1 (true) or 0 (false), default = 0)\n"
/* Always assumed true!!
"-region_nghbrs_list	(bool)		Flag to request the inclusion of the list\n"
"					of the region classes neighboring each\n"
"					region class.\n"
"					(1 (true) or 0 (false), default = 0)\n"
*/
"-region_nb_objects	(bool)		Flag to request the inclusion of the\n"
"					number of region objects contained in\n"
"					each region class. (1 (true) or\n"
"					 0 (false), default = 1 (true)\n"
"					 if spclust_wght != 0.0 and both\n"
"					 \"-object_labels_map\" and\n"
"					 \"-region_objects\"are specified,\n"
"					 and false otherwise.\n"
"					 A true value is allowed only when\n"
"					 spclust_wght != 0.0 and both\n"
"					 \"-object_labels_map\" and\n"
"					 \"-region_objects\" are specified.\n"
"					 User provided value ignored and set to\n"
"					 1 (true) if \"-region_objects_list\"\n"
"					 (below) is true.)\n"
"-region_objects_list	(bool)		Flag to request the inclusion of the\n"
"					list of region objects contained in\n"
"					each region class. (1 (true) or\n"
"					 0 (false), default = 1 (true) if\n"
"					 spclust_wght != 0.0 and both\n"
"					 \"-object_labels_map\" and\n"
"					 \"-region_objects\" are specified,\n"
"					 and 0 (false) otherwise.\n"
"					 A true value is allowed only when\n"
"					 spclust_wght != 0.0 and both\n"
"					 \"-object_labels_map\" and\n"
"					 \"-region_objects\" are specified.)\n");

  fprintf(stdout,"\nThe following parameters are recommended for variation by all users\n");
  fprintf(stdout,"(defaults provided):\n\n");
  fprintf(stdout,"-std_dev_wght		(float)		Weight for standard deviation spatial\n"
"					feature (std_dev_wght >= 0.0,\n"
"					 default = 0.0.  Ignored if\n"
"					 -std_dev_image not provided,\n"
"					 and for dissim_crit #5 and #9)\n"
"-edge_threshold		(float)		Threshold for initial neighborhood\n"
"					merges based on edge information.\n"
"					(0.0 <= edge_threshold < 0.5,\n"
"					 default = %3.2f if an edge_image)\n"
"					 provided. If an edge_image is\n"
"					 not provided, the user supplied\n"
"					 value is ignored, and edge_threshold\n"
"					 is set to 0.0)\n"
"-edge_wght		(float)		Relative weight for edge information\n"
"					in calculating neighboring region\n"
"					dissimilarity. (0.0 <= edge_wght <\n"
"					 1.0, default = %3.2f if an edge_image\n"
"					 is provided. If an edge_image is\n"
"					 not provided, the user supplied\n"
"					 value is ignored, and edge_wght\n"
"					 is set to 0.0)\n"
"-edge_power		(float)		The power to which the edge_value\n"
"					feature is raised after it is scaled\n"
"					to a range of 0 to 1 and before it is\n"
"					used to modify the HSeg region\n"
"					dissimilarity value. (Default = 1.0)\n"
"-seam_edge_threshold	(float)		Threshold for across seam merges\n"
"					based on edge information for processing\n"
"					window artifact elimination.\n"
"					(0.0 <= seam_edge_threshold < 0.5,\n"
"					 default = %3.2f. Required in RHSEG\n"
"					 mode, but ignored otherwise)\n"
"-conn_type		(int)		Neighbor connectivity type\n"
"					  1-D Case:\n"
"					    1. \"Two Nearest Neighbors\",\n"
"					    2. \"Four Nearest Neighbors\",\n"
"					    3. \"Six Nearest Neighbors\",\n"
"					    4. \"Eight Nearest Neighbors\",\n"
"					    (default: 1. \"Two Nearest\n"
"					     Neighbors\")\n"
"					  2-D Case:\n"
"					    1. \"Four Nearest Neighbors\",\n"
"					    2. \"Eight Nearest Neighbors\",\n"
"					    3. \"Twelve Nearest Neighbors\",\n"
"					    4. \"Twenty Nearest Neighbors\",\n"
"					    5. \"Twenty-Four Nearest Neighbors\",\n"
"					    (default: 2. \"Eight Nearest\n"
"					     Neighbors\")\n"
#ifdef THREEDIM
"					  3-D Case:\n"
"					    1. \"Six Nearest Neighbors\",\n"
"					    2. \"Eighteen Nearest Neighbors\",\n"
"					    3. \"Twenty-Six Nearest Neighbors\",\n"
"					    (default: 3. \"Twenty-Six Nearest\n"
"					     Neighbors\")\n"
#endif
"-chk_nregions  		(unsigned int)	Number of regions at which hierarchical\n"
"					segmentation output is initiated (2 <=\n"
"					 chk_nregions<%d, default calculated\n"
"					 if \"hseg_out_nregions\" and\n"
"					 \"hseg_out_thresholds\" not specified)\n"
"-hseg_out_nregions 	(unsigned int)	The set of number of region levels\n"
"					at which hierarchical segmentation\n"
"					outputs are made (a comma delimited\n"
"					 list)\n"
"-hseg_out_thresholds	(float)		The set of merge thresholds at which\n"
"					hierarchical segmentation outputs are\n"
"					made (a comma delimited list)\n"
"NOTE: -chk_nregions, -hseg_out_nregions and -hseg_out_thresholds are mutually\n"
"exclusive. If more than one of these are specified, the last one specified\n"
"controls and the previous specifications are ignored. However,\n"
"-hseg_out_nregions and -hseg_out_thresholds may not be specified for\n"
"-rnb_levels > 1.\n\n"
"-conv_nregions		(unsigned int)	Number of regions for final convergence\n"
"					(0 < conv_nregions < %d, default=%d)\n"
"-gdissim		(bool)		Flag to request output of global\n"
"					dissimilarity values in the log file.\n"
"					(1 (true) or 0 (false), default = 0)\n",
EDGE_THRESHOLD, EDGE_WGHT, SEAM_EDGE_THRESHOLD, USHRT_MAX, USHRT_MAX, CONV_NREGIONS);

  fprintf(stdout,"\nThe default values should be used for the following parameters,\n");
  fprintf(stdout,"except in special circumstances (defaults provided):\n\n");
  fprintf(stdout,"-debug			(int)		Debug option - controls log file\n"
"					outputs, (0 < debug < 255, default = 1)\n"
"-normind		(int)		Image normalization type\n"
"					  1.\"No Normalization\",\n"
"					  2.\"Normalize Across Bands\",\n"
"					  3.\"Normalize Bands Separately\"\n"
"					(default: 2. \"Normalize Across Bands\")\n"
"-init_threshold		(float)		Threshold for initial fast region\n"
"					merging by a region oriented first\n"
"					merge process (default = 0.0)\n"
"					NOTE: Ignored is edge_threshold > 0.0\n"
"					and edge_image is provided.\n"
"-initial_merge_npix	(unsigned int)	Suppress neighbor merges between regions\n"
"					that are larger than this number of\n"
"					pixels after the initial fast region\n"
"					merging (default: no suppression)\n"
"-random_init_seed	(bool)		Flag to request a \"random\" seed for the\n"
"					sampling procedure utilized in the\n"
"					initial fast region merging process\n"
"					(1 (true) or 0 (false), default = 0)\n"
"-sort			(bool)		Flag to request that the region classes\n"
"					and region objects be sorted.\n"
"					(1 (true) or 0 (false), default = 1)\n"
"-edge_dissim_option	(int)		Edge dissimilarity option\n"
"					  1.\"Merge enhancement option\",\n"
"					  2.\"Merge suppression option\",\n"
"					(default: 1. \"Merge enhancement option\"\n"
"					 for HSWO mode, otherwise default is\n"
"					 2. \"Merge suppression option\"\n"
"					 NOTE: option 2 not allowed in HSWO mode)\n"
"-rnb_levels		(int)		Number of recursive levels\n"
"					(0 < rnb_levels < 255, default\n"
"					 calculated for RHSEG program mode.\n"
"					 Value ignored and parameter set to 1\n"
"					 for HSWO and HSEG program modes.)\n"
"-ionb_levels		(int)		Recursive level at which data I/O is\n"
"					performed (0 < ionb_levels <=\n"
"					 rnb_levels, default calculated)\n"
"-min_nregions		(unsigned int)	Number of regions for convergence at\n"
"					intermediate stages.\n");
   cout << "					(0 < min_nregions < " << UINT_MAX << " default" << endl;
   fprintf(stdout,"					 calculated for RHSEG program mode)\n"
"-spclust_min		(unsigned int)	Minimum number of regions for which\n"
"					spectral clustering is utilized\n"
"					(For splcust_wght = 0.0, default = 0;\n"
"					 otherwise default = %d)\n"
"-spclust_max		(unsigned int)	Maximum number of regions for which\n"
"					spectral clustering is utilized\n"
"					(For splcust_wght = 0.0, default = 0;\n"
"					 otherwise default = %d)\n"
"-merge_acceleration	(bool)		Flag to request utilization of a merge\n"
"					acceleration factor for small regions.\n"
"					(1 (true) or 0 (false), default = 0)\n",SPCLUST_MIN,SPCLUST_MAX);

#if (defined(WINDOWS) || defined(CYGWIN))
  cout << endl << "Copyright (C) 2006 United States Government as represented by the" << endl;
#else
  cout << endl << "Copyright \u00a9 2006 United States Government as represented by the" << endl;
#endif
  cout << "Administrator of the National Aeronautics and Space Administration." << endl;
  cout << "No copyright is claimed in the United States under Title 17, U.S. Code." << endl;
  cout << "All Other Rights Reserved." << endl << endl;

  return;
}
