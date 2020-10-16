// Global function definitions
int broadcast_params();

void do_class_label_offset(const short unsigned int& recur_level,
                      unsigned int class_label_offset, vector<Pixel>& pixel_data,
                      Temp& temp_data);

void do_object_label_offset(const short unsigned int& recur_level,
                           unsigned int object_label_offset, Spatial& spatial_data,
                           Temp& temp_data);

unsigned int recur_receive(const short unsigned int& section, const short unsigned int& recur_level,
                           unsigned int class_label_offset, vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                           unsigned int& global_nregions, double& max_threshold, 
                           Temp& temp_data);

void recur_send(const short unsigned int& section, const short unsigned int& recur_level,
                vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, 
                const unsigned int& nregions, const unsigned int& global_nregions, 
                const double& max_threshold, Temp& temp_data);
#ifdef THREEDIM
void parallel_recur_requests(const short unsigned int& request_id, const short unsigned int& recur_level,
                             unsigned int value, const int& ncols, const int& nrows, const int& nslices,
                             unsigned int short_buf_size, unsigned int int_buf_size,
                             unsigned int double_buf_size, Temp& temp_data);

void parallel_recur_request(const short unsigned int& request_id, const short unsigned int& recur_level,
                            const int& recur_taskid, unsigned int value1, unsigned int value2,
                            const int& ncols, const int& nrows, const int& nslices,
                            unsigned int int_buf_size, unsigned int double_buf_size,
                            Temp& temp_data);
#else
void parallel_recur_requests(const short unsigned int& request_id, const short unsigned int& recur_level,
                             unsigned int value, const int& ncols, const int& nrows,
                             unsigned int short_buf_size, unsigned int int_buf_size,
                             unsigned int double_buf_size, Temp& temp_data);

void parallel_recur_request(const short unsigned int& request_id, const short unsigned int& recur_level,
                            const int& recur_taskid, unsigned int value1, unsigned int value2,
                            const int& ncols, const int& nrows,
                            unsigned int int_buf_size, unsigned int double_buf_size,
                            Temp& temp_data);
#endif
void parallel_request(const short unsigned int& request_id,
                      const int& onb_taskid, const unsigned int& value,
                      unsigned int byte_buf_size, unsigned int short_buf_size,
                      unsigned int int_buf_size, unsigned int double_buf_size,
                      Temp& temp_data);

void parallel_server(const short unsigned int& section, Spatial& spatial_data, vector<Pixel>& pixel_data,
                     vector<RegionClass>& region_classes, vector<RegionClass *>& nghbr_heap, vector<RegionClass *>& region_heap,
                     Temp& temp_data);

void do_termination(const short unsigned int& recur_level, Temp& temp_data);
