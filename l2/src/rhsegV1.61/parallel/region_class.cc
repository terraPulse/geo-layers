 void RegionClass::set_all_data(Temp& temp_data, unsigned int class_label_offset, unsigned int& short_buf_position,
                                unsigned int& int_buf_position, unsigned int& float_buf_position, unsigned int& double_buf_position)
 {
    int band;
    unsigned int nghbrs_label_size, nghbrs_index;

    initial_merge_flag = (temp_data.short_buffer[short_buf_position++] == 1);
    npix = temp_data.int_buffer[int_buf_position++];
    nghbrs_label_size = temp_data.int_buffer[int_buf_position++];
    for (band = 0; band < nbands; ++band)
    {
      sum[band] = temp_data.double_buffer[double_buf_position++];
      if (sumsq_flag)
        sumsq[band] = temp_data.double_buffer[double_buf_position++];
      if (sumxlogx_flag)
        sumxlogx[band] = temp_data.double_buffer[double_buf_position++];
      if (std_dev_flag)
        sum_pixel_std_dev[band] = temp_data.double_buffer[double_buf_position++];
    }
    if (params.edge_image_flag)
      max_edge_value = temp_data.float_buffer[float_buf_position++];
    merge_threshold = temp_data.double_buffer[double_buf_position++];

    nghbrs_label_set.clear();
    for (nghbrs_index = 0; nghbrs_index < nghbrs_label_size; nghbrs_index++)
      nghbrs_label_set.insert(temp_data.int_buffer[int_buf_position++] + class_label_offset);
    return;
 }

 void RegionClass::set_data(Temp& temp_data, unsigned int& int_buf_position, unsigned int& double_buf_position)
 {
    int band;

    npix = temp_data.int_buffer[int_buf_position++];
    for (band = 0; band < nbands; ++band)
    {
      sum[band] = temp_data.double_buffer[double_buf_position++];
      if (sumsq_flag)
        sumsq[band] = temp_data.double_buffer[double_buf_position++];
      if (sumxlogx_flag)
        sumxlogx[band] = temp_data.double_buffer[double_buf_position++];
      if (std_dev_flag)
        sum_pixel_std_dev[band] = temp_data.double_buffer[double_buf_position++];
    }

    return;
 }

 void RegionClass::update_all_data(Temp& temp_data, unsigned int& int_buf_position, 
                                   unsigned int& float_buf_position, unsigned int& double_buf_position)
 {
    int band;
    unsigned int nghbrs_index, nghbrs_label_size;
    float    temp_float;
    double   temp_double;

//    initial_merge_flag = (temp_data.short_buffer[short_buf_position++] == 1);  Don't need this here!
    npix += temp_data.int_buffer[int_buf_position++];
    nghbrs_label_size = temp_data.int_buffer[int_buf_position++];  // nghbrs_label_set.size()
    for (band = 0; band < nbands; ++band)
    {
      sum[band] += temp_data.double_buffer[double_buf_position++];
      if (sumsq_flag)
        sumsq[band] += temp_data.double_buffer[double_buf_position++];
      if (sumxlogx_flag)
        sumxlogx[band] += temp_data.double_buffer[double_buf_position++];
      if (std_dev_flag)
        sum_pixel_std_dev[band] = +temp_data.double_buffer[double_buf_position++];
    }
    if (params.edge_image_flag)
    {
      temp_float = temp_data.float_buffer[float_buf_position++];
      if (temp_float > max_edge_value)
        max_edge_value = temp_float;
    }
    temp_double = temp_data.double_buffer[double_buf_position++];
    if (temp_double > merge_threshold)
      merge_threshold = temp_double;

    for (nghbrs_index = 0; nghbrs_index < nghbrs_label_size; nghbrs_index++)
      nghbrs_label_set.insert(temp_data.int_buffer[int_buf_position++]);

    return;
 }

 void RegionClass::update_region_info(Temp& temp_data, unsigned int& int_buf_position)
 {
    unsigned int region_object, nb_objects;

    boundary_npix += temp_data.int_buffer[int_buf_position++];
    if (params.region_nb_objects_flag)
    {
      nb_objects = temp_data.int_buffer[int_buf_position++];
      for (region_object = 0; region_object < nb_objects; ++region_object)
        region_objects_set.insert(temp_data.int_buffer[int_buf_position++]);
    }

    return;
 }

 void RegionClass::load_all_data(Temp& temp_data, unsigned int& short_buf_position, unsigned int& int_buf_position,
                                 unsigned int& float_buf_position, unsigned int& double_buf_position)
 {
   int band;
   int nghbrs_label_size = nghbrs_label_set.size( );

   if (initial_merge_flag)
     temp_data.short_buffer[short_buf_position++] = 1;
   else
     temp_data.short_buffer[short_buf_position++] = 0;
   temp_data.int_buffer[int_buf_position++] = npix;
   temp_data.int_buffer[int_buf_position++] = (unsigned int) nghbrs_label_size;
   for (band = 0; band < nbands; ++band)
   {
     temp_data.double_buffer[double_buf_position++] = sum[band];
     if (sumsq_flag)
       temp_data.double_buffer[double_buf_position++] = sumsq[band];
     if (sumxlogx_flag)
       temp_data.double_buffer[double_buf_position++] = sumxlogx[band];
     if (std_dev_flag)
       temp_data.double_buffer[double_buf_position++] = sum_pixel_std_dev[band];
   }
   if (params.edge_image_flag)
     temp_data.float_buffer[float_buf_position++] = max_edge_value;
   temp_data.double_buffer[double_buf_position++] = merge_threshold;

   if (nghbrs_label_size > 0)
   {
     set<unsigned int>::const_iterator nghbrs_label_iter = nghbrs_label_set.begin( );
     while (nghbrs_label_iter != nghbrs_label_set.end( ))
     {
       temp_data.int_buffer[int_buf_position++] = *nghbrs_label_iter;
       ++nghbrs_label_iter;
     }
   }

   return;
 }

 void RegionClass::load_data(Temp& temp_data, unsigned int& int_buf_position, unsigned int& double_buf_position)
 {
   int band;

   temp_data.int_buffer[int_buf_position++] = npix;
   for (band = 0; band < nbands; ++band)
   {
     temp_data.double_buffer[double_buf_position++] = sum[band];
     if (sumsq_flag)
       temp_data.double_buffer[double_buf_position++] = sumsq[band];
     if (sumxlogx_flag)
       temp_data.double_buffer[double_buf_position++] = sumxlogx[band];
     if (std_dev_flag)
       temp_data.double_buffer[double_buf_position++] = sum_pixel_std_dev[band];
   }

   return;
 }

 void RegionClass::get_region_info(Temp& temp_data, unsigned int& int_buf_position)
 {
    unsigned int nb_objects;

    temp_data.int_buffer[int_buf_position++] = boundary_npix;
    if (params.region_nb_objects_flag)
    {
      nb_objects = region_objects_set.size( );
      temp_data.int_buffer[int_buf_position++] = nb_objects;
      set<unsigned int>::const_iterator region_object_iter = region_objects_set.begin( );
      while (region_object_iter != region_objects_set.end( ))
      {
        temp_data.int_buffer[int_buf_position++] = *region_object_iter;
        ++region_object_iter;
      }
    }

    return;

 }
