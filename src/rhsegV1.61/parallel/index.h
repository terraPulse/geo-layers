      public:
         //  MODIFICATION MEMBER FUNCTIONS
          void set_data(bool spatial_data_flag, Temp& temp_data,
                        unsigned int& short_buf_position, unsigned int& int_buf_position, unsigned int& float_buf_position);
         //  CONSTANT MEMBER FUNCTIONS
          void load_data(bool spatial_data_flag, Temp& temp_data,
                         unsigned int& short_buf_position, unsigned int& int_buf_position, unsigned int& float_buf_position);
