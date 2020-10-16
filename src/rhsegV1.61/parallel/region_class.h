    public:
     //  MODIFICATION MEMBER FUNCTIONS
      void set_all_data(Temp& temp_data, unsigned int class_label_offset, unsigned int& short_buf_position,
                        unsigned int& int_buf_position, unsigned int& float_buf_position, unsigned int& double_buf_position);
      void set_data(Temp& temp_data, unsigned int& int_buf_position, unsigned int& double_buf_position);
      void update_all_data(Temp& temp_data, unsigned int& int_buf_position, 
                           unsigned int& float_buf_position, unsigned int& double_buf_position);
      void update_region_info(Temp& temp_data, unsigned int& int_buf_position);
      void update_sum_pixel_gdissim(double value)
           { sum_pixel_gdissim += value; }
     //  CONSTANT MEMBER FUNCTIONS
      void load_all_data(Temp& temp_data, unsigned int& short_buf_position, unsigned int& int_buf_position,
                         unsigned int& float_buf_position, unsigned int& double_buf_position);
      void load_data(Temp& temp_data, unsigned int& int_buf_position, unsigned int& double_buf_position);
      void get_region_info(Temp& temp_data, unsigned int& int_buf_position);
