// params.cc

#include "params.h"
#include <iostream>
#include <cstdlib>

#include <iostream>
#include <ctime>

CommonTilton::Image inputImage;
CommonTilton::Image maskImage;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;

    mask_flag = false;
    mask_value = 0;
    bias_value = 0;
    fc_option = 1;
    edge_threshold = 0.0;
    output_type = 1;
    scale_output_flag = false;

    return;
  }

 // Destructor...
  Params::~Params() 
  {
    return;
  }

 // Print version
  void Params::print_version( )
  {
    cout << endl << version << endl << endl;

    return;
  }

 // Read parameters
  bool Params::read(const char *param_file)
  {
//    cout << "Reading parameters from " << param_file << endl;
    ifstream param_fs(param_file);
    if (!param_fs.is_open())
    {
      cout << "Cannot open input file " << param_file << endl;
      return false;
    }

  // Read initial parameters from parameter file until End-of-File reached
#ifdef GDAL
    int    band;
    double min_value, max_value;
#endif
    bool input_image_flag = false;
    bool ncols_flag = false;
    bool nrows_flag = false;
    bool nbands_flag = false;
    bool dtype_flag = false;
    bool mask_value_flag = false;
    bool edge_operation_flag = false;
    bool output_image_flag = false;
    bool output_mask_image_flag = false;
    int sub_pos;
    string line, sub_string;
    while (!param_fs.eof())
    {
      getline(param_fs,line);
      sub_pos = line.find("-");
      while ((!param_fs.eof()) && (sub_pos != 0))
      {
        getline(param_fs,line);
        sub_pos = line.find("-");
      }
      if (line.find("-input_image") != string::npos)
      {
        input_image_file = process_line(line,false);
#ifdef GDAL
        input_image_flag = inputImage.open(input_image_file);
        if (input_image_flag)
        {
          ncols = inputImage.get_ncols();
          ncols_flag = true;
          nrows = inputImage.get_nrows();
          nrows_flag = true;
          nbands = inputImage.get_nbands();
          nbands_flag = true;
          dtype = inputImage.get_dtype();
          dtype_flag = true;
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
                                cout << "WARNING: For the input image data file " << input_image_file << "," << endl;
                                cout << "short integer data will be converted to 32-bit float data." << endl;
                                dtype = Float32;
                              }
                              else
                              {
                                cout << "WARNING: For the input image data file " << input_image_file << "," << endl;
                                cout << "short integer data will be converted to unsigned short integer data." << endl;
                                dtype = UInt16;
                              }
                              break;
            case GDT_UInt32:  cout << "NOTE: For the input image data file " << input_image_file << "," << endl;
                              cout << "32-bit unsigned integer data will be converted to 32-bit float data." << endl;
                              dtype = Float32;
                              break;
            case GDT_Int32:   cout << "NOTE: For the input image data file " << input_image_file << "," << endl;
                              cout << "32-bit integer data will be converted to 32-bit float data." << endl;
                              dtype = Float32;
                              break;
            case GDT_Float32: break;
            case GDT_Float64: cout << "WARNING: For the input image data file " << input_image_file << "," << endl;
                              cout << "64-bit double data will be converted to 32-bit float data." << endl;
                              cout << "Out of ranges value will not be read properly." << endl;
                              dtype = Float32;
                              break;
            default:          cout << "Unknown or unsupported image data type for input image data file ";
                              cout << input_image_file << endl;
                              return false;
          } 
        }
        else
        {
          cout << "ERROR:  Input image file " << input_image_file << " is of unknown format." << endl;
        }
#else
        input_image_flag = true;
#endif
      }
#ifndef GDAL
      if (line.find("-ncols") != string::npos)
      {
        sub_string = process_line(line,false);
        ncols = atoi(sub_string.c_str());
        ncols_flag = true;
      }
      if (line.find("-nrows") != string::npos)
      {
        sub_string = process_line(line,false);
        nrows = atoi(sub_string.c_str());
        nrows_flag = true;
      }
      if (line.find("-nbands") != string::npos)
      {
        sub_string = process_line(line,false);
        nbands = atoi(sub_string.c_str());
        nbands_flag = true;
      }
      if (line.find("-dtype") != string::npos)
      {
        sub_string = process_line(line,false);
        if (sub_string == "UInt8")
        {
          dtype = UInt8;
          dtype_flag = true;
        }
        else if (sub_string == "UInt16")
        {
          dtype = UInt16;
          dtype_flag = true;
        }
        else if (sub_string == "Float32")
        {
          dtype = Float32;
          dtype_flag = true;
        }
        else
        { 
          dtype = Unknown;
          dtype_flag = false;
        }
      }
#endif
      if ((line.find("-mask") != string::npos) && (line.find("-mask_value") == string::npos))
      {
        mask_file = process_line(line,false);
#ifdef GDAL
        mask_flag = maskImage.open(mask_file);
        if (mask_flag)
        {
          min_value = 0;
          max_value = 255;
          if (min_value > maskImage.getMinimum(0))
            min_value = maskImage.getMinimum(0);
          if (max_value < maskImage.getMaximum(0))
            max_value = maskImage.getMaximum(0);
          if ((min_value < 0.0) || (max_value > 255))
          {
            cout << "ERROR: Invalid range for input mask image. Must be in range 0 to 255." << endl;
            mask_flag = false;
            return false;
          }
          if ((!mask_value_flag) && (maskImage.no_data_value_valid(0)))
          {
            min_value = maskImage.get_no_data_value(0);
            if ((min_value < 256.0) && (min_value >= 0.0))
              mask_value = (unsigned char) min_value;
            else
            {
              cout << "ERROR: Mask value, " << mask_value << ", is out of 8-bit range" << endl;
              mask_flag = false;
              return false;
            }
          }
        }
        else
        {
          cout << "WARNING:  Input mask file " << mask_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
        }
#else
        mask_flag = true;
#endif
      }
      if (line.find("-mask_value") != string::npos)
      {
        sub_string = process_line(line,false);
        mask_value = atoi(sub_string.c_str());
        mask_value_flag = true;
      }
      if (line.find("-bias_value") != string::npos)
      {
        sub_string = process_line(line,false);
        bias_value = atoi(sub_string.c_str());
      }
      if (line.find("-edge_operation") != string::npos)
      {
        sub_string = process_line(line,false);
        if (sub_string == "Prewitt")
        {
          edge_operation = Prewitt;
          edge_operation_flag = true;
        }
        else if (sub_string == "Sobel")
        {
          edge_operation = Sobel;
          edge_operation_flag = true;
        }
        else if (sub_string == "Scharr")
        {
          edge_operation = Scharr;
          edge_operation_flag = true;
        }
        else if (sub_string == "Frei-Chen")
        {
          edge_operation = Frei_Chen;
          edge_operation_flag = true;
        }
        else if (sub_string == "Std_Dev")
        {
          edge_operation = Std_Dev;
          edge_operation_flag = true;
        }
        else
        { 
          edge_operation_flag = false;
        }
      }
      if (line.find("-Frei-Chen_option") != string::npos)
      {
        sub_string = process_line(line,false);
        fc_option = atoi(sub_string.c_str());
      }
      if (line.find("-edge_threshold") != string::npos)
      {
        sub_string = process_line(line,false);
        edge_threshold = (float) atof(sub_string.c_str());
      }
      if (line.find("-output_type") != string::npos)
      {
        sub_string = process_line(line,false);
        output_type = atoi(sub_string.c_str());
      }
      if (line.find("-scale_output") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          scale_output_flag = true;
        else
          scale_output_flag = false;
      }
      if (line.find("-output_image") != string::npos)
      {
        output_image_file = process_line(line,false);
        output_image_flag = true;
      }
      if (line.find("-output_mask_image") != string::npos)
      {
        output_mask_image_file = process_line(line,false);
        output_mask_image_flag = true;
      }
    }

    param_fs.close();

// Exit with false status if a required parameter is not set, or an improper parameter
// setting is detected.
    if (!input_image_flag)
    {
      cout << "ERROR: -input_image (Input image file) is required" << endl;
      return false;
    }
    if (!ncols_flag)
    {
      cout << "ERROR: -ncols (Number of columns in input image data) is required" << endl;
      return false;
    }
    if (!nrows_flag)
    {
      cout << "ERROR: -nrows (Number of rows in input image data) is required" << endl;
      return false;
    }
    if (!nbands_flag)
    {
      cout << "ERROR: -nbands (Number of spectral bands in input image data) is required" << endl;
      return false;
    }
    if (!dtype_flag)
    {
      cout << "ERROR: -dtype (Data type of input image data) is required" << endl;
      return false;
    }
    if (!edge_operation_flag)
    {
      cout << "ERROR: -edge_operation (Edge operation type) is required" << endl;
      return false;
    }
    if ((fc_option < 1) or (fc_option > 2))
    {
      cout << "ERROR: Invalid Frei-Chen_option. -Frei-Chen_option must be 1 or 2" << endl;
      return false;
    }
    if ((output_type < 1) or (output_type > 4))
    {
      cout << "ERROR: Invalid output_type (output type). -output_type must be 1, 2, 3 or 4" << endl;
      return false;
    }
    if (!output_image_flag)
    {
      cout << "ERROR: -output_image (Output image file) is required" << endl;
      return false;
    }
    if (!output_mask_image_flag)
    {
      cout << "ERROR: -output_mask_image (Output mask image file) is required" << endl;
      return false;
    }
#ifndef GDAL
    if (!inputImage.open(input_image_file,ncols,nrows,nbands,dtype))
    {
      cout << "ERROR: Could not open -input_image: " << input_image_file << endl;
      return false;
    }
    if (mask_flag)
    {
      if (!maskImage.open(mask_file,ncols,nrows,nbands,UInt8))
      {
        cout << "ERROR: Could not open -mask: " << mask_file << endl;
        return false;
      }
    }
#endif

    if (edge_operation == Std_Dev)
    {
     // Need temporary file names for this case
      const char *temp_dir;
      temp_dir = getenv("TMP");
      if (temp_dir == NULL)
        temp_dir = getenv("TEMP");
      if (temp_dir == NULL)
        temp_dir = getenv("TMPDIR");
      if (temp_dir == NULL)
      {
        temp_dir = (char *) malloc(5*sizeof(char));
        string tmp = "/tmp";
        temp_dir = tmp.c_str();
      }
      string temp_directory = temp_dir;

      static char time_buffer[26];
      time_t now;
      const  struct tm *tm_ptr;
      now = time(NULL);
      tm_ptr = localtime(&now);
/*
      int length;
      length = strftime(time_buffer,26,"%a%b%d%H%M%S%Y",tm_ptr);
*/
      strftime(time_buffer,26,"%a%b%d%H%M%S%Y",tm_ptr);

      string test_file_name = time_buffer;
#ifdef WINDOWS
      temp_file_name = temp_directory + "\\EDGE_" + test_file_name;
#else
      temp_file_name = temp_directory + "/EDGE_" + test_file_name;
#endif
      test_file_name = temp_file_name + ".test_open";
      fstream io_file_fs;
      io_file_fs.open(test_file_name.c_str( ), ios_base::out );
      if (!io_file_fs)
      {
        cout << "ERROR:  Failed to open temporary file: " << test_file_name << endl;
        return false;
      }
      io_file_fs.close( );
      remove(test_file_name.c_str());
    }

    return true;
  }

 // Print parameters
  void Params::print()
  {
  // Print version
      cout << "This is " << version << endl << endl;

  // Print input parameters
      cout << "Input image file name: " << input_image_file << endl;
#ifndef GDAL
      cout << "Input image number of columns = " <<  ncols << endl;
      cout << "Input image number of rows = " <<  nrows << endl;
      cout << "Input image number of bands = " <<  nbands << endl;
      switch (dtype)
      {
        case UInt8:   cout << "Input image data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  cout << "Input image data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: cout << "Input image data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      cout << "WARNING: Input image data type in invalid (Unknown)" << endl;
                      break;
      }
#endif
      if (bias_value != 0)
        cout << "Bias value = " << bias_value << endl;
      switch (edge_operation)
      {
        case Prewitt:   cout << "Requested Prewitt edge operation" << endl;
                        break;
        case Sobel:     cout << "Requested Sobel edge operation" << endl;
                        break;
        case Scharr:    cout << "Requested Scharr edge operation" << endl;
                        break;
        case Frei_Chen: cout << "Requested Frei-Chen edge operation" << endl;
                        switch (fc_option)
                        {
                          case 1:  cout << "Requested edge Frei-Chen option" << endl;
                                   break;
                          case 2:  cout << "Requested line-edge Frei-Chen option" << endl;
                                   break;
                          default: cout << "WARNING: Requested invalid Frei-Chen option" << endl;
                                   break;
                        }
                        break;
        case Std_Dev:   cout << "Requested Standard Deviation operation" << endl;
                        break;
        default:        cout << "WARNING: Requested invalid edge operation (Unknown)" << endl;
                        break;
      }
      if (edge_threshold > 0.0)
        cout << "Edge threshold = " << edge_threshold << endl;
      switch (output_type)
      {
        case 1:  cout << "Requested multispectral output" << endl;
                 break;
        case 2:  cout << "Requested band average output" << endl;
                 break;
        case 3:  cout << "Requested band maximum output" << endl;
                 break;
        case 4:  cout << "Requested band minimum output" << endl;
                 break;
        default: cout << "WARNING: Invalid output_type" << endl;
                 break;
      }
      if (scale_output_flag)
        cout << "Scaling of the edge values requested" << endl;
      cout << "Output image file name: " << output_image_file << endl;
      cout << "Output mask image file name: " << output_mask_image_file << endl;

      return;
  }

  string process_line(const string& line, const bool& list_flag)
  {
    int sub_pos, sub_pos_space, sub_pos_tab;
    string sub_string;

    sub_pos_space = line.find_first_of(" ");
    sub_pos_tab = line.find_first_of("\t");
    sub_pos = sub_pos_space;
    if (sub_pos ==  (int) string::npos)
      sub_pos = sub_pos_tab;
    if (sub_pos ==  (int) string::npos)
      return " ";

    sub_string = line.substr(sub_pos);
    while ((sub_string.substr(0,1) == "\t") || (sub_string.substr(0,1) == " "))
      sub_string = line.substr(++sub_pos);
#ifndef WINDOWS
    if (list_flag)
      return sub_string;

    sub_pos = sub_string.find_first_of(" ");
    if (sub_pos !=  (int) string::npos)
      sub_string = sub_string.substr(0,sub_pos);
    sub_pos = sub_string.find_first_of("\t");
    if (sub_pos !=  (int) string::npos)
      sub_string = sub_string.substr(0,sub_pos);
    sub_pos = sub_string.find_first_of("\r");
    if (sub_pos !=  (int) string::npos)
      sub_string = sub_string.substr(0,sub_pos);
#endif
    return sub_string;
  }

} // namespace CommonTilton
