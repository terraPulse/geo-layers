 void Index::set_data(bool spatial_data_flag, Temp& temp_data,
                      unsigned int& short_buf_position, unsigned int& int_buf_position, unsigned int& float_buf_position)
 {
   pixel_index = temp_data.int_buffer[int_buf_position++];
   pixel_section = temp_data.short_buffer[short_buf_position++];
   if (spatial_data_flag)
   {
     region_object_label = temp_data.int_buffer[int_buf_position++];
     boundary_map = temp_data.short_buffer[short_buf_position++];
   }
   else
   {
     edge_mask = (temp_data.float_buffer[float_buf_position] > -FLT_MAX);
     edge_value = temp_data.float_buffer[float_buf_position++];
   }
   region_class_label = temp_data.int_buffer[int_buf_position++];

   return;
 }

 void Index::load_data(bool spatial_data_flag, Temp& temp_data,
                       unsigned int& short_buf_position, unsigned int& int_buf_position, unsigned int& float_buf_position)
 {
   temp_data.int_buffer[int_buf_position++] = pixel_index;
   temp_data.short_buffer[short_buf_position++] = pixel_section;
   if (spatial_data_flag)
   {
     temp_data.int_buffer[int_buf_position++] = region_object_label;
     temp_data.short_buffer[short_buf_position++] = boundary_map;
   }
   else
   {
     if (edge_mask)
       temp_data.float_buffer[float_buf_position++] = edge_value;
     else
       temp_data.float_buffer[float_buf_position++] = -FLT_MAX;
   }
   temp_data.int_buffer[int_buf_position++] = region_class_label;

   return;
 }
