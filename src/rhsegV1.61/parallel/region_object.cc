 void RegionObject::set_data(Temp& temp_data,
                             unsigned int& int_buf_position, unsigned int& double_buf_position)
 {
    short unsigned int band;

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

 void RegionObject::update_data(Temp& temp_data,
                                unsigned int& int_buf_position, unsigned int& double_buf_position)
 {
    short unsigned int band;

    npix += temp_data.int_buffer[int_buf_position++];
    for (band = 0; band < nbands; ++band)
    {
      sum[band] += temp_data.double_buffer[double_buf_position++];
      if (sumsq_flag)
        sumsq[band] += temp_data.double_buffer[double_buf_position++];
      if (sumxlogx_flag)
        sumxlogx[band] += temp_data.double_buffer[double_buf_position++];
      if (std_dev_flag)
        sum_pixel_std_dev[band] += temp_data.double_buffer[double_buf_position++];
    }

    return;
 }

 void RegionObject::update_boundary_npix(Temp& temp_data, unsigned int& int_buf_position)
 {
    boundary_npix += temp_data.int_buffer[int_buf_position++];

    return;
 }

  void RegionObject::load_data(Temp& temp_data,
                               unsigned int& int_buf_position, unsigned int& double_buf_position)
 {
   short unsigned int band;

   temp_data.int_buffer[int_buf_position++] = label;
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

 void RegionObject::load_boundary_npix(Temp& temp_data, unsigned int& int_buf_position)
 {
    temp_data.int_buffer[int_buf_position++] = boundary_npix;

    return;

 }

