 void Pixel::set_all_data(Temp& temp_data, unsigned int class_label_offset, unsigned int& byte_buf_position, 
                          unsigned int& int_buf_position, unsigned int& float_buf_position)
 {
   short unsigned int band;

   init_flag = true;
   region_label = class_label_offset + temp_data.int_buffer[int_buf_position++];
   for (band = 0; band < nbands; ++band)
     set_input_data(band,temp_data.float_buffer[float_buf_position++]);
   if (std_dev_flag)
   {
     std_dev_mask = (temp_data.byte_buffer[byte_buf_position++] == 1);
     for (band = 0; band < nbands; ++band)
       set_local_std_dev(band,temp_data.float_buffer[float_buf_position++]);
   }
   if (params.edge_image_flag)
   {
     edge_mask = (temp_data.byte_buffer[byte_buf_position++] == 1);
     set_edge_value(temp_data.float_buffer[float_buf_position++]);
   }

   return;
 }

  void Pixel::load_all_data(Temp& temp_data, unsigned int& byte_buf_position, 
                            unsigned int& int_buf_position, unsigned int& float_buf_position)
 {
   short unsigned int band;

   temp_data.int_buffer[int_buf_position++] = region_label;
   for (band = 0; band < nbands; ++band)
     temp_data.float_buffer[float_buf_position++] = get_input_data(band);
   if (std_dev_flag)
   {
     if (std_dev_mask)
       temp_data.byte_buffer[byte_buf_position++] = 1;
     else
       temp_data.byte_buffer[byte_buf_position++] = 0;
     for (band = 0; band < nbands; ++band)
       temp_data.float_buffer[float_buf_position++] = get_local_std_dev(band);
   }
   if (params.edge_image_flag)
   {
     if (edge_mask)
       temp_data.byte_buffer[byte_buf_position++] = 1;
     else
       temp_data.byte_buffer[byte_buf_position++] = 0;
     temp_data.float_buffer[float_buf_position++] = get_edge_value();
   }

   return;
 }

