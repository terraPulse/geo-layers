    public:
     //  MODIFICATION MEMBER FUNCTIONS
      void set_all_data(Temp& temp_data, unsigned int class_label_offset, unsigned int& byte_buf_position, 
                        unsigned int& int_buf_position, unsigned int& float_buf_position);
     //  CONSTANT MEMBER FUNCTIONS
      void load_all_data(Temp& temp_data, unsigned int& byte_buf_position, 
                         unsigned int& int_buf_position, unsigned int& float_buf_position);
