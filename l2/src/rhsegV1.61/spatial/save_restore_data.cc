/*-----------------------------------------------------------
|
|  Routine Name: save_restore_data - Routines for saving and restoring data from temporary files.
|
|       Purpose: Routines for saving and restoring data from temporary files.
|
|         Input: file_type        (Designation of type of data saved or restored)
|                section          (Data section to be saved or restored)
|                nelements        (Number of data elements to be saved or restored)
|                data_buffer      (Data buffer holding data to be saved or to received data restored)
|
|        Output:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: May 27, 2005.
| Modifications: May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|
------------------------------------------------------------*/

#include <params/params.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
  void save_byte_data(const string file_type,
                      const short unsigned int& section, const unsigned int& nelements,
                      unsigned char *byte_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::out | ios_base::binary );
    io_file_fs.write(reinterpret_cast<char *>(byte_buffer),nelements);
    io_file_fs.close( );

    return;
  }

  void save_short_data(const string file_type,
                       const short unsigned int& section, const unsigned int& nelements,
                       short unsigned int *short_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::out | ios_base::binary );
    io_file_fs.write(reinterpret_cast<char *>(short_buffer),2*nelements);
    io_file_fs.close( );

    return;
  }

  void save_int_data(const string file_type,
                     const short unsigned int& section, const unsigned int& nelements,
                     unsigned int *int_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::out | ios_base::binary );
    io_file_fs.write(reinterpret_cast<char *>(int_buffer),4*nelements);
    io_file_fs.close( );

    return;
  }

  void save_float_data(const string file_type,
                      const short unsigned int& section, const unsigned int& nelements,
                      float *float_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::out | ios_base::binary );
    io_file_fs.write(reinterpret_cast<char *>(float_buffer),4*nelements);
    io_file_fs.close( );

    return;
  }

  void restore_byte_data(const string file_type,
                         const short unsigned int& section, const unsigned int& nelements,
                         unsigned char *byte_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::in | ios_base::binary );
    io_file_fs.read(reinterpret_cast<char *>(byte_buffer),nelements);
    io_file_fs.close( );

    return;
  }

  void restore_short_data(const string file_type,
                          const short unsigned int& section, const unsigned int& nelements,
                          short unsigned int *short_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::in | ios_base::binary );
    io_file_fs.read(reinterpret_cast<char *>(short_buffer),2*nelements);
    io_file_fs.close( );

    return;
  }

  void restore_int_data(const string file_type,
                        const short unsigned int& section, const unsigned int& nelements,
                        unsigned int *int_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::in | ios_base::binary );
    io_file_fs.read(reinterpret_cast<char *>(int_buffer),4*nelements);
    io_file_fs.close( );

    return;
  }

  void restore_float_data(const string file_type,
                        const short unsigned int& section, const unsigned int& nelements,
                        float *float_buffer)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    fstream io_file_fs;
    io_file_fs.open(io_file_name.c_str( ), ios_base::in | ios_base::binary );
    io_file_fs.read(reinterpret_cast<char *>(float_buffer),4*nelements);
    io_file_fs.close( );

    return;
  }

  void remove_temp_file(const string file_type,
                        const short unsigned int& section)
  {
    char *char_section;
    char_section = (char *) malloc(4*sizeof(char));

    string temp_file_name;
    temp_file_name = params.temp_file_name + "." + file_type + "_sec";

    string io_file_name;
    sprintf(char_section,"%03d",section);
    io_file_name = temp_file_name + char_section;

    remove(io_file_name.c_str( ));

    return;
  }
} // namespace HSEGTilton

