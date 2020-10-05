/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>> 	Main program for the rhseg_setup program
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
   >>>>                October 6, 2009: Added robustness to the reading of parameter values from parameter files
   >>>>                November 4, 2009 - Added automatic mask creation
   >>>>		       January 10, 2010: Changed the definition of min_npixels parameter
   >>>>		       January 19, 2010: Changed the definition of spclust_start parameter and renamed it spclust_max
   >>>>                May 26, 2010: Default redefined for split_pixels_factor for program modes HSWO and HSEG.
   >>>>                June 7, 2010: Modified operation of the RHSEG program mode, eliminating the need for the max_min_npixels parameter.
   >>>>                September 10, 2010 - Reintroduced optional small region merge acceleration factor.
   >>>>	               December 28, 2010 - Added spclust_min parameter.
   >>>>                May 9, 2011: Added the output of the nb_sections parameter, which indicates the number of processes for parallel processing.
   >>>>                May 9, 2011: Added a Makefile for a non-GDAL, 3D version of rhseg_setup (rhseg_setup_3d).
   >>>>                February 24, 2012 - Upgraded boundary_map from Byte to Short Integer
   >>>>		       March 1, 2013 - Expanded the use of the standard deviation spatial feature.
   >>>>                May 22, 2013 - Added edge input image (edgeInImage) along with the RegionEdge class and associated code to optionally 
   >>>>                               utilize edge information to influence the region growing process
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
#include <params/params.h>
#ifdef GTKMM
#include <params/initialGUI.h>
#include <params/outputFileGUI.h>
#include <params/paramsGUI.h>
#endif
#ifdef GDAL
#include <image/image.h>
#include <gdal_priv.h>
#endif
#include <iostream>
#include <cstring>
#ifdef GTKMM
#include <gtkmm.h>
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

#ifdef GDAL
extern Image inputImage;
extern Image maskImage;
extern Image stdDevInImage;
extern Image stdDevMaskImage;
extern Image edgeInImage;
extern Image edgeMaskImage;
extern Image regionMapInImage;
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
|  Input from command line (no input from command line for GUI version) -
|       Serial version:     parameter_file_name
|     Parallel version:     parameter_file_name inb_levels onb_levels
|
|       Returns: EXIT_SUCCESS on success, EXIT_FAILURE on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: January 28, 2009.
| Modifications: May 12, 2009 - Modified user interface and data I/O for compatibility with ILIADS project needs.
|                July 8, 2009 - Final clean-up of code for release
|                November 4, 2009 - Added automatic mask creation
|
------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  int status = true;

#ifdef GTKMM
  Gtk::Main kit(argc, argv);
  Glib::ustring strTime = " ";
  Glib::ustring strMessage = " ";
#endif

#ifdef GDAL
#ifdef THREEDIM
#ifdef GTKMM
  if (argc == 1)
  {
    strMessage += "\nGDAL may not be used with three-dimensional processing.\nUse rhseg_run directly.\n";
    Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    end_dialog.run();
  }
  else
#endif // GTKMM
    cout << "ERROR: GDAL may not be used with three-dimensional processing.\n       Use rhseg_run directly." << endl;
  return EXIT_FAILURE;
#endif // THREEDIM
  GDALAllRegister();
#endif // GDAL

  if (argc == 1)
  {
#ifdef GTKMM
    params.gtkmm_flag = true;
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
      return EXIT_SUCCESS;
    if (status == 2)
    {
      if (params.debug > 0)
      {
        params.log_fs.open(params.log_file.c_str(),ios_base::out);
        status = params.log_fs.is_open();
      }
    }
#else
    usage();
    cout << "ERROR: Need parameter file as argument." << endl;
    return EXIT_FAILURE;
#endif
  }
  else if ((strncmp(argv[1],"-v",2) == 0) || (strncmp(argv[1],"-version",8) == 0))
  {
    params.print_version();
    cout << "For help information: rhseg_setup -h or rhseg_setup -help" << endl << endl;
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
        return EXIT_FAILURE;
      }
      if (status)
        status = params.read(argv[1]);
      else
      {
        usage();
        cout << "ERROR: Error opening log file: " << params.log_file << endl;
        return EXIT_FAILURE;
      }
    }
  }

  if (status)
  {
// Print program parameters
    if (params.debug > 0)
      params.print();

#ifdef GDAL
    if (params.gdal_input_flag)
    {
      Image inputImageENVI;
      params.input_image_file = params.current_folder + params.prefix + INPUT_IMAGE_FILE;
      if (inputImageENVI.create_copy(params.input_image_file,inputImage,"ENVI"))
        cout << "Successfully created copy of inputImage to " << params.input_image_file << endl;
      else
      {
        int band, ncols, nrows, nbands;
        GDALDataType data_type;
        double min_value;
        ncols = inputImage.get_ncols();
        nrows = inputImage.get_nrows();
        nbands = inputImage.get_nbands();
        data_type = inputImage.get_data_type();
        switch (data_type)
        {
          case GDT_Byte:    break;
          case GDT_UInt16:  break;
          case GDT_Int16:   min_value = 0;
                            for (band = 0; band < nbands; band++)
                              if (min_value > inputImage.getMinimum(band))
                                min_value = inputImage.getMinimum(band);
                            if (min_value < 0.0)
                            {
                              cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                              cout << "short integer data will be converted to 32-bit float data." << endl;
                              data_type = GDT_Float32;
                            }
                            else
                            {
                              cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                              cout << "short integer data will be converted to unsigned short integer data." << endl;
                              data_type = GDT_UInt16;
                            }
                            break;
          case GDT_UInt32:  cout << "NOTE: For the input image data file " << params.input_image_file << "," << endl;
                            cout << "32-bit unsigned integer data will be converted to 32-bit float data." << endl;
                            data_type = GDT_Float32;
                            break;
          case GDT_Int32:   cout << "NOTE: For the input image data file " << params.input_image_file << "," << endl;
                            cout << "32-bit integer data will be converted to 32-bit float data." << endl;
                            data_type = GDT_Float32;
                            break;
          case GDT_Float32: break;
          case GDT_Float64: cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                            cout << "64-bit double data will be converted to 32-bit float data." << endl;
                            cout << "Out of ranges value will not be read properly." << endl;
                            data_type = GDT_Float32;
                            break;
          default:          cout << "Unknown or unsupported image data type for input image data file ";
                            cout << params.input_image_file << endl;
                            return false;
        }
        string driver_description = "ENVI";  // Use ENVI format for rhseg_setup!!
        inputImageENVI.create(params.input_image_file,ncols,nrows,nbands,data_type,driver_description);
        GDALDataset  *inputDataset, *outputDataset;
        inputDataset = inputImage.get_imageDataset();
        outputDataset = inputImageENVI.get_imageDataset();
        float *buffer = new float[ncols*nrows];
        for (band = 0; band < params.nbands; band++)
        {
          cout << "Processing band " << (band + 1) << endl;
          GDALRasterBand *rb = inputDataset->GetRasterBand(band+1);
          GDALRasterBand *wb = outputDataset->GetRasterBand(band+1);
          rb->RasterIO(GF_Read, 0, 0, ncols, nrows, buffer, ncols, nrows, GDT_Float32, 0, 0);
          wb->RasterIO(GF_Write, 0, 0, ncols, nrows, buffer, ncols, nrows, GDT_Float32, 0, 0);
        }
      }
      inputImageENVI.close();

      if (params.mask_flag)
      {
        Image maskImageENVI;
        params.mask_file = params.current_folder + params.prefix + MASK_FILE;
        maskImageENVI.create_copy(params.mask_file ,maskImage,"ENVI");
        maskImageENVI.close();
      }
      else if (inputImage.no_data_value_valid(0))
      {
        params.mask_flag = true;
        Image maskImageENVI;
        params.mask_file = params.current_folder + params.prefix + MASK_FILE;
        maskImageENVI.create_mask(params.mask_file ,inputImage,"ENVI");
        maskImageENVI.close();
      }

      if (params.std_dev_image_flag)
      {
        Image stdDevInImageENVI;
        params.std_dev_image_file = params.current_folder + params.prefix + STD_DEV_IN_FILE;
        stdDevInImageENVI.create_copy(params.std_dev_image_file,stdDevInImage,"ENVI");
        stdDevInImageENVI.close();
      }

      if (params.std_dev_mask_flag)
      {
        Image stdDevMaskImageENVI;
        params.std_dev_mask_file = params.current_folder + params.prefix + STD_DEV_MASK_FILE;
        stdDevMaskImageENVI.create_copy(params.std_dev_mask_file,stdDevMaskImage,"ENVI");
        stdDevMaskImageENVI.close();
      }

      if (params.edge_image_flag)
      {
        Image edgeInImageENVI;
        params.edge_image_file = params.current_folder + params.prefix + EDGE_IN_FILE;
        edgeInImageENVI.create_copy(params.edge_image_file,edgeInImage,"ENVI");
        edgeInImageENVI.close();
      }

      if (params.edge_mask_flag)
      {
        Image edgeMaskImageENVI;
        params.edge_mask_file = params.current_folder + params.prefix + EDGE_MASK_FILE;
        edgeMaskImageENVI.create_copy(params.edge_mask_file,edgeMaskImage,"ENVI");
        edgeMaskImageENVI.close();
      }

      if (params.region_map_in_flag)
      {
        Image regionMapInImageENVI;
        params.region_map_in_file = params.current_folder + params.prefix + REGION_MAP_IN_FILE;
        regionMapInImageENVI.create_copy(params.region_map_in_file,regionMapInImage,"ENVI");
        regionMapInImageENVI.close();
      }
    }
#endif

    params.write_oparam();

#ifdef GDAL
    if (params.gdal_input_flag)
    {
    // create classLabelsMapImage, boundaryMapImage and objectLabelsMapImage to get the ENVI headers
      int nbands = 1;
      GDALDataType data_type = GDT_UInt32;
      string driver_description = "ENVI";  // Use ENVI format for rhseg_setup!!
      Image classLabelsMapImage, boundaryMapImage, objectLabelsMapImage;
      classLabelsMapImage.create(params.class_labels_map_file,inputImage,nbands,data_type,driver_description);
      classLabelsMapImage.close();
      if (params.boundary_map_flag)
      {
        data_type = GDT_UInt16;
        boundaryMapImage.create(params.boundary_map_file,inputImage,nbands,data_type,driver_description);
        boundaryMapImage.close();
      }
      if (params.object_labels_map_flag)
      {
        data_type = GDT_UInt32;
        objectLabelsMapImage.create(params.object_labels_map_file,inputImage,nbands,data_type,driver_description);
        objectLabelsMapImage.close();
      }
    }
#endif
  }

#ifdef GDAL
  if (params.gdal_input_flag)
  {
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
  }
#endif

  if (status)
  {
    if (params.debug > 0)
      params.log_fs << endl << "Successful completion of rhseg_setup" << endl;
    if (!params.gtkmm_flag)
    {
      cout << endl << "Successful completion of rhseg_setup" << endl;
    }
#ifdef GTKMM
    else
    {
      strMessage += "\nSuccessful completion of rhseg_setup";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);

      end_dialog.run();
    }
#endif
  }
  else
  {
    if (params.debug > 0)
      params.log_fs << endl << "The rhseg_setup program terminated improperly." << endl;
    if (!params.gtkmm_flag)
      cout << endl << "The rhseg_setup program terminated improperly." << endl;
#ifdef GTKMM
    else
    {
      strMessage = "\nThe hseg/rhseg program terminated improperly\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
    }
#endif
  }

  if (params.log_fs.is_open())
    params.log_fs.close( );

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
|       Written: January 28, 2009.
| Modifications:
|
------------------------------------------------------------*/
void usage()
{
  cout << endl << "Usage: " << endl << endl;
#ifdef GTKMM
  cout << "rhseg_setup" << endl;
  cout << "or" << endl;
#endif
  cout << "rhseg_setup parameter_file_name" << endl << endl;
  cout << "For help information: rhseg_setup -h or rhseg_setup -help" << endl;
  cout << "For version information: rhseg_setup -v or rhseg_setup -version" << endl;

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
|       Written: January 28, 2009.
| Modifications: May 12, 2009: Modified user interface and data I/O for compatibility with ILIADS project needs.
|
------------------------------------------------------------*/
void help()
{
#ifdef GTKMM
  cout << endl << "The rhseg_setup program can be called in two different manners:" << endl;
  cout << endl << "rhseg_setup" << endl;
  cout << endl << "(where the program parameters are entered via a Graphical User Interface)" << endl;
  cout << endl << "or" << endl;
  cout << endl << "rhseg_setup parameter_file_name" << endl;
  cout << endl << "(where the program parameters are read from a parameter file)" << endl;
  cout << endl << "In the later case, \"parameter_file_name\" is the name of the input parameter" << endl;
#else
  cout << endl << "The rhseg_setup program is called in the following manner:" << endl;
  cout << endl << "rhseg_setup parameter_file_name" << endl;
  cout << endl << "where \"parameter_file_name\" is the name of the input parameter" << endl;
#endif // GTKMM
  cout << "file. For contents see below." << endl;
  cout << endl << "For this help: rhseg_setup -h or rhseg_setup -help" << endl;
  cout << endl << "For version information: rhseg_setup -v or rhseg_setup -version";
  cout << endl;
  cout << endl << "The (optional) parameter file consists of entries of the form:" << endl;
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
#ifndef GDAL
"-ncols			(int)		Number of columns in input image data\n"
"				        (0 < ncols < %d, required)\n"
"-nrows			(int)		Number of rows in input image data\n"
"				        (0 < nrows < %d, required)\n"
#ifdef THREEDIM
"-nslices		(int)		Number of slices in input image data\n"
"					(0 < nslices < %d, required)\n"
#endif
"-nbands			(int)		Number of spectral bands in input image\n"
"					data.  (0 < nbands < %d, required)\n"
"-dtype			(string)	Input image data type. Currently must\n"
"					be either:\n"
"					 UInt8   => \"unsigned char (8-bit)\" or\n"
"					 UInt16  => \"unsigned short int\n"
"						     (16-bit)\" or\n"
"					 Float32 => \"float (32-bit)\"\n"
#ifdef THREEDIM
"					(required)\n",USHRT_MAX,USHRT_MAX,USHRT_MAX,USHRT_MAX);
#else
"					(required)\n",USHRT_MAX,USHRT_MAX,USHRT_MAX);
#endif
  fprintf(stdout,"-scale			(double)	Comma delimited list of input data\n"
"					scale factors (specify one value per\n"
"					 band, default = 1.0 for each band)\n"
"-offset			(double)	Comma delimited list of input data\n"
"					offset factors (specify one value per\n"
"					 band, default = 0.0 for each band)\n");

  fprintf(stdout,"-mask			(string)	Input data mask file name\n"
"					Data type must be unsigned char.\n"
"					(default = {none})\n"
"-mask_value		(int)		If input data mask file is provided,\n"
"					this is the value in the mask file that\n"
"					designates bad data. (default = 0.)\n"
#else
"-mask			(string)	Input data mask file name (optional,\n"
"					 default = {none})\n"
#endif
"-std_dev_image		(string)	Input standard deviation image file\n"
"					(optional, default = {none})\n"
"-std_dev_mask		(string)	Input standard devation mask file\n"
"					(optional, default = {none},\n"
"					 0 designates bad data)\n"
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
