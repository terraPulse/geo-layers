    public:
     //  MODIFICATION MEMBER FUNCTIONS
      void set_data(Temp& temp_data,
                    unsigned int& int_buf_position, unsigned int& double_buf_position);
      void update_data(Temp& temp_data,
                       unsigned int& int_buf_position, unsigned int& double_buf_position);
      void update_boundary_npix(Temp& temp_data, unsigned int& int_buf_position);
     //  CONSTANT MEMBER FUNCTIONS
      void load_data(Temp& temp_data,
                     unsigned int& int_buf_position, unsigned int& double_buf_position);
      void load_boundary_npix(Temp& temp_data, unsigned int& int_buf_position);
