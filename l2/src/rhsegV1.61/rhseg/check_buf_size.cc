/*-----------------------------------------------------------
|
|  Routine Name: check_buf_size
|
|       Purpose: Checks the temp_data buffer sizes and readjusts them as necessary
|
|         Input: byte_buf_size    (Size required for byte_buffer)
|                short_buf_size   (Size required for short_buffer)
|                int_buf_size     (Size required for int_buffer)
|                float_buf_size   (Size required for float_buffer)
|                double_buf_size  (Size required for double_buffer)
|
|         Other: temp_data        (Data buffers)
|
|        Output:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: November 12, 2003
| Modifications: June 17, 2005 - Added temporary file I/O for faster processing of large data sets
|                February 8, 2006 - Added the float_buffer
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
   void check_buf_size(const unsigned int& byte_buf_size,
                       const unsigned int& short_buf_size, const unsigned int& int_buf_size,
                       const unsigned int& float_buf_size, const unsigned int& double_buf_size,
                       Temp& temp_data)
   {
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << "Checking byte_buf_size = " << byte_buf_size;
       params.log_fs << ", short_buf_size = " << short_buf_size;
       params.log_fs << ", int_buf_size = " << int_buf_size;
       params.log_fs << ", float_buf_size = " << float_buf_size;
       params.log_fs << " and double_buf_size = " << double_buf_size << endl;
     }
#endif

     if (byte_buf_size > temp_data.byte_buf_size)
     {
       if (params.debug > 3)
       {
         params.log_fs << "byte_buf_size readjusted from " << temp_data.byte_buf_size;
         params.log_fs << " to " << byte_buf_size << endl;
       }
       if (temp_data.byte_buffer != NULL)
         delete [ ] temp_data.byte_buffer;
       temp_data.byte_buffer = new unsigned char[byte_buf_size];
       temp_data.byte_buf_size = byte_buf_size;
     }

     if (short_buf_size > temp_data.short_buf_size)
     {
       if (params.debug > 3)
       {
         params.log_fs << "short_buf_size readjusted from " << temp_data.short_buf_size;
         params.log_fs << " to " << short_buf_size << endl;
       }
       if (temp_data.short_buffer != NULL)
         delete [ ] temp_data.short_buffer;
       temp_data.short_buffer = new short unsigned int[short_buf_size];
       temp_data.short_buf_size = short_buf_size;
     }

     if (int_buf_size > temp_data.int_buf_size)
     {
       if (params.debug > 3)
       {
         params.log_fs << "int_buf_size readjusted from " << temp_data.int_buf_size;
         params.log_fs << " to " << int_buf_size << endl;
       }
       if (temp_data.int_buffer != NULL)
         delete [ ] temp_data.int_buffer;
       temp_data.int_buffer = new unsigned int[int_buf_size];
       temp_data.int_buf_size = int_buf_size;
     }

     if (float_buf_size > temp_data.float_buf_size)
     {
       if (params.debug > 3)
       {
         params.log_fs << "float_buf_size readjusted from " << temp_data.float_buf_size;
         params.log_fs << " to " << float_buf_size << endl;
       }
       if (temp_data.float_buffer != NULL)
         delete [ ] temp_data.float_buffer;
       temp_data.float_buffer = new float[float_buf_size];
       temp_data.float_buf_size = float_buf_size;
     }

     if (double_buf_size > temp_data.double_buf_size)
     {
       if (params.debug > 3)
       {
         params.log_fs << "double_buf_size readjusted from " << temp_data.double_buf_size;
         params.log_fs << " to " << double_buf_size << endl;
       }
       if (temp_data.double_buffer != NULL)
         delete [ ] temp_data.double_buffer;
       temp_data.double_buffer = new double[double_buf_size];
       temp_data.double_buf_size = double_buf_size;
     }

#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << "Exited check_buf_size with byte_buf_size = " << byte_buf_size;
       params.log_fs << ", short_buf_size = " << short_buf_size;
       params.log_fs << ", int_buf_size = " << int_buf_size;
       params.log_fs << ", float_buf_size = " << float_buf_size;
       params.log_fs << " and double_buf_size = " << double_buf_size << endl;
     }
#endif
   }
} // namespace HSEGTilton
