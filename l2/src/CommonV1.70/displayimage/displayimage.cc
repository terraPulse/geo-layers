// displayimage.cc

#include "displayimage.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

namespace CommonTilton
{

  bool DisplayImage::circle_roi_highlight_all_flag;
  bool DisplayImage::geotransform_flag;
  bool DisplayImage::hseglearn_flag;
  int DisplayImage::input_ncols;
  int DisplayImage::input_nrows;
  int DisplayImage::base_ncols;
  int DisplayImage::base_nrows;
  double DisplayImage::base_X_gsd;
  double DisplayImage::base_Y_gsd;
  double DisplayImage::X_offset;
  double DisplayImage::Y_offset;
  double DisplayImage::X_zoom_factor;
  double DisplayImage::Y_zoom_factor;
  string DisplayImage::current_folder;
  string DisplayImage::shortcut_folder;

  DisplayImage::DisplayImage()
  {
    display_type = 0;
    button_press_monitor_flag = false;
    select_region_flag = false;
    circle_roi_flag = false;
    button_pressed_flag = false;
    cross_hair_flag = false;
    numberLinked = 0;
    hadjustment_value = 0.0;
    vadjustment_value = 0.0;
    colPosition = 0;
    rowPosition = 0;
    colClick = 0;
    rowClick = 0;
    clickFlag = false;
    resizeFlag = true;

    return;
  }

  DisplayImage::~DisplayImage()
  {
    return;
  }

  void DisplayImage::set_static_values(const Image& input_image, const string& folder)
  {
    bool highlight_all_flag = false;
    set_static_values(input_image, folder, highlight_all_flag);

    return;
  }

  void DisplayImage::set_static_values(const Image& input_image, const string& folder, const bool& highlight_all_flag)
  {
    bool hlearn_flag = false;
    set_static_values(input_image, folder, highlight_all_flag, hlearn_flag);

    return;
  }

  void DisplayImage::set_static_values(const Image& input_image, const string& folder, const bool& highlight_all_flag, const bool& hlearn_flag)
  {
    circle_roi_highlight_all_flag = highlight_all_flag;
    hseglearn_flag = hlearn_flag;

    geotransform_flag = input_image.geotransform_valid();

    if (geotransform_flag)
    {
      base_X_gsd = input_image.get_X_gsd();
      base_Y_gsd = input_image.get_Y_gsd();
      if (fabs(base_X_gsd) >= fabs(base_Y_gsd))
      {
        X_zoom_factor = fabs(base_X_gsd)/fabs(base_Y_gsd);
        Y_zoom_factor = 1.0;
      }
      else
      {
        X_zoom_factor = 1.0;
        Y_zoom_factor = fabs(base_Y_gsd)/fabs(base_X_gsd);
      }
      base_X_gsd = base_X_gsd/X_zoom_factor;
      base_Y_gsd = base_Y_gsd/Y_zoom_factor;

      input_ncols = input_image.get_ncols();
      input_nrows = input_image.get_nrows();
      base_ncols = (int) ((input_ncols*X_zoom_factor) + 0.5);
      base_nrows = (int) ((input_nrows*Y_zoom_factor) + 0.5);
      X_offset = input_image.get_X_offset();
      Y_offset = input_image.get_Y_offset();
    }
    else
    {
      base_X_gsd = 1;
      base_Y_gsd = 1;
      X_zoom_factor = 1.0;
      Y_zoom_factor = 1.0;
      input_ncols = (int) input_image.get_ncols();
      input_nrows = (int) input_image.get_nrows();
      base_ncols = input_ncols;
      base_nrows = input_nrows;
      X_offset = 0.0;
      Y_offset = 0.0;
    }

    current_folder = folder;
    shortcut_folder = folder;

    return;
  }

  void DisplayImage::init_rgb(Image& input_image, const Glib::ustring& title)
  {
    Image mask_image;
    init_rgb(input_image, mask_image, title);

    return;
  }

  void DisplayImage::init_rgb(Image& input_image, Image& mask_image, const Glib::ustring& title)
  {
    bool mask_flag, histo_flag;
    int  col, row, band, nbands;
    double min_value, max_value;
    double red_value, green_value, blue_value;
    double red_scale, green_scale, blue_scale;
    double red_offset, green_offset, blue_offset;
    int histo_size, histo_max_index, histo_min_index;

    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = &mask_image;
    mask_valid = maskImage->data_valid();

    path.clear();

    display_type = 1;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    nbands = inputImage->get_nbands();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);
/*
cout << endl << "In init_rgb, input_ncols = " << input_ncols << ", and input_nrows = " << input_nrows << endl;
cout << "inputImage_ncols = " << inputImage->get_ncols() << ", and inputImage_nrows = " << inputImage->get_nrows() << endl;
cout << "relative_X_zoom_factor = " << relative_X_zoom_factor << " and relative_Y_zoom_factor = " << relative_Y_zoom_factor << endl;
*/
    band_min = new double[nbands];
    band_max = new double[nbands];
    for (band = 0; band < nbands; band++)
    {
      band_min[band] = inputImage->getMinimum(band,mask_image);
      band_max[band] = inputImage->getMaximum(band,mask_image);
    }

    inputImage->get_rgb_image_stretch(rgb_image_stretch,range[0],range[1]);
    histo_flag = false;
    if (rgb_image_stretch != 1)
      histo_flag = ((inputImage->histo_eq_map_valid(inputImage->get_red_display_band())) &&
                    (inputImage->histo_eq_map_valid(inputImage->get_green_display_band())) &&
                    (inputImage->histo_eq_map_valid(inputImage->get_blue_display_band())));
    if (!histo_flag)
      rgb_image_stretch = 1;

    red_scale = 1.0;
    green_scale = 1.0;
    blue_scale = 1.0;
    red_offset = 0.0;
    green_offset = 0.0; 
    blue_offset = 0.0;
    if (rgb_image_stretch != 2)
    {
      band = inputImage->get_red_display_band();
      if (rgb_image_stretch == 1)
      {
        min_value = band_min[band]*range[0];
        max_value = band_max[band]*range[1];
      }
      else
      {
        histo_size = (double) inputImage->get_histo_eq_map_size(band);
        histo_min_index = histo_size*range[0];
        histo_max_index = histo_size*range[1];
        min_value = inputImage->get_histo_eq_map(band,histo_min_index);
        max_value = inputImage->get_histo_eq_map(band,histo_max_index);
      }
      if (max_value > min_value)
      {
        red_scale = 255.0/(max_value - min_value);
        red_offset = min_value;
      }
      band = inputImage->get_green_display_band();
      if (rgb_image_stretch == 1)
      {
        min_value = band_min[band]*range[0];
        max_value = band_max[band]*range[1];
      }
      else
      {
        histo_size = (double) inputImage->get_histo_eq_map_size(band);
        histo_min_index = histo_size*range[0];
        histo_max_index = histo_size*range[1];
        min_value = inputImage->get_histo_eq_map(band,histo_min_index);
        max_value = inputImage->get_histo_eq_map(band,histo_max_index);
      }
      if (max_value > min_value)
      {
        green_scale = 255.0/(max_value - min_value);
        green_offset = min_value;
      }
      band = inputImage->get_blue_display_band();
      if (rgb_image_stretch == 1)
      {
        min_value = band_min[band]*range[0];
        max_value = band_max[band]*range[1];
      }
      else
      {
        histo_size = (double) inputImage->get_histo_eq_map_size(band);
        histo_min_index = histo_size*range[0];
        histo_max_index = histo_size*range[1];
        min_value = inputImage->get_histo_eq_map(band,histo_min_index);
        max_value = inputImage->get_histo_eq_map(band,histo_max_index);
      }
      if (max_value > min_value)
      {
        blue_scale = 255.0/(max_value - min_value);
        blue_offset = min_value;
      }
    }

    set_display_file();
    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        red_value = 0;
        green_value = 0;
        blue_value = 0;
        if (inputImage->data_valid(col,row,inputImage->get_red_display_band()) &&
            inputImage->data_valid(col,row,inputImage->get_green_display_band()) &&
            inputImage->data_valid(col,row,inputImage->get_blue_display_band()))
        {
          mask_flag = true;
          if (mask_valid)
            mask_flag = maskImage->data_valid(col,row,0);
          if (mask_flag)
          {
            red_value = inputImage->get_data(col,row,inputImage->get_red_display_band());
            green_value = inputImage->get_data(col,row,inputImage->get_green_display_band());
            blue_value = inputImage->get_data(col,row,inputImage->get_blue_display_band());
            if (rgb_image_stretch == 2)
            {
              red_value = inputImage->get_histo_lookup(red_value,inputImage->get_red_display_band());
              green_value = inputImage->get_histo_lookup(green_value,inputImage->get_green_display_band());
              blue_value = inputImage->get_histo_lookup(blue_value,inputImage->get_blue_display_band());
            }
            else
            {
              red_value = red_scale*(red_value-red_offset);
              green_value = green_scale*(green_value-green_offset);
              blue_value = blue_scale*(blue_value-blue_offset);
            }
          }
        }
        if (red_value > 255.0)
          red_value = 255.0;
        if (green_value > 255.0)
          green_value = 255.0;
        if (blue_value > 255.0)
          blue_value = 255.0;
        if (red_value < 0.0)
          red_value = 0.0;
        if (green_value < 0.0)
          green_value = 0.0;
        if (blue_value < 0.0)
          blue_value = 0.0;
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    set_title(title);
    init();

    return;
  }

  void DisplayImage::reinit_rgb()
  {
    bool mask_flag, histo_flag;
    int  col, row, band;
    double min_value, max_value;
    double red_value, green_value, blue_value;
    double red_scale, green_scale, blue_scale;
    double red_offset, green_offset, blue_offset;
    int histo_size, histo_max_index, histo_min_index;

    path.clear();

    inputImage->get_rgb_image_stretch(rgb_image_stretch,range[0],range[1]);
    histo_flag = false;
    if (rgb_image_stretch != 1)
      histo_flag = ((inputImage->histo_eq_map_valid(inputImage->get_red_display_band())) &&
                    (inputImage->histo_eq_map_valid(inputImage->get_green_display_band())) &&
                    (inputImage->histo_eq_map_valid(inputImage->get_blue_display_band())));
    if (!histo_flag)
      rgb_image_stretch = 1;

    red_scale = 1.0;
    green_scale = 1.0;
    blue_scale = 1.0;
    red_offset = 0.0;
    green_offset = 0.0; 
    blue_offset = 0.0;
    if (rgb_image_stretch != 2)
    {
      band = inputImage->get_red_display_band();
      if (rgb_image_stretch == 1)
      {
        min_value = band_min[band]*range[0];
        max_value = band_max[band]*range[1];
      }
      else
      {
        histo_size = (double) inputImage->get_histo_eq_map_size(band);
        histo_min_index = histo_size*range[0];
        histo_max_index = histo_size*range[1];
        min_value = inputImage->get_histo_eq_map(band,histo_min_index);
        max_value = inputImage->get_histo_eq_map(band,histo_max_index);
      }
      if (max_value > min_value)
      {
        red_scale = 255.0/(max_value - min_value);
        red_offset = min_value;
      }
      band = inputImage->get_green_display_band();
      if (rgb_image_stretch == 1)
      {
        min_value = band_min[band]*range[0];
        max_value = band_max[band]*range[1];
      }
      else
      {
        histo_size = (double) inputImage->get_histo_eq_map_size(band);
        histo_min_index = histo_size*range[0];
        histo_max_index = histo_size*range[1];
        min_value = inputImage->get_histo_eq_map(band,histo_min_index);
        max_value = inputImage->get_histo_eq_map(band,histo_max_index);
      }
      if (max_value > min_value)
      {
        green_scale = 255.0/(max_value - min_value);
        green_offset = min_value;
      }
      band = inputImage->get_blue_display_band();
      if (rgb_image_stretch == 1)
      {
        min_value = band_min[band]*range[0];
        max_value = band_max[band]*range[1];
      }
      else
      {
        histo_size = (double) inputImage->get_histo_eq_map_size(band);
        histo_min_index = histo_size*range[0];
        histo_max_index = histo_size*range[1];
        min_value = inputImage->get_histo_eq_map(band,histo_min_index);
        max_value = inputImage->get_histo_eq_map(band,histo_max_index);
      }
      if (max_value > min_value)
      {
        blue_scale = 255.0/(max_value - min_value);
        blue_offset = min_value;
      }
    }

    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        red_value = 0;
        green_value = 0;
        blue_value = 0;
        if (inputImage->data_valid(col,row,inputImage->get_red_display_band()) &&
            inputImage->data_valid(col,row,inputImage->get_green_display_band()) &&
            inputImage->data_valid(col,row,inputImage->get_blue_display_band()))
        {
          mask_flag = true;
          if (mask_valid)
            mask_flag = maskImage->data_valid(col,row,0);
          if (mask_flag)
          {
            red_value = inputImage->get_data(col,row,inputImage->get_red_display_band());
            green_value = inputImage->get_data(col,row,inputImage->get_green_display_band());
            blue_value = inputImage->get_data(col,row,inputImage->get_blue_display_band());
            if (rgb_image_stretch == 2)
            {
              red_value = inputImage->get_histo_lookup(red_value,inputImage->get_red_display_band());
              green_value = inputImage->get_histo_lookup(green_value,inputImage->get_green_display_band());
              blue_value = inputImage->get_histo_lookup(blue_value,inputImage->get_blue_display_band());
            }
            else
            {
              red_value = red_scale*(red_value-red_offset);
              green_value = green_scale*(green_value-green_offset);
              blue_value = blue_scale*(blue_value-blue_offset);
            }
          }
        }
        if (red_value > 255.0)
          red_value = 255.0;
        if (green_value > 255.0)
          green_value = 255.0;
        if (blue_value > 255.0)
          blue_value = 255.0;
        if (red_value < 0.0)
          red_value = 0.0;
        if (green_value < 0.0)
          green_value = 0.0;
        if (blue_value < 0.0)
          blue_value = 0.0;
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    reset_display_thumb_pixbuf();

    return;
  }

  void DisplayImage::init_seg(Image& input_image, const Glib::ustring& title)
  {
    int col, row;
    unsigned int data_value;
    unsigned char red_value, green_value, blue_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = NULL;
    mask_valid = false;

    path.clear();

    display_type = 2;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    set_display_file();
    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        data_value = (unsigned int) inputImage->get_data(col,row,0);
        red_value = inputImage->get_red_colormap(data_value);
        green_value =inputImage->get_green_colormap(data_value);
        blue_value = inputImage->get_blue_colormap(data_value);
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    set_title(title);

    init();

    return;
  }

  void DisplayImage::reinit_seg()
  {
    int col, row;
    unsigned int data_value;
    unsigned char red_value, green_value, blue_value;

    path.clear();

    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        data_value = (unsigned int) inputImage->get_data(col,row,0);
        red_value = inputImage->get_red_colormap(data_value);
        green_value =inputImage->get_green_colormap(data_value);
        blue_value = inputImage->get_blue_colormap(data_value);
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    reset_display_thumb_pixbuf();

    return;
  }

  void DisplayImage::init_region_label(Image& input_image, const Glib::ustring& title)
  {
    Image mask_image;

    init_region_label(input_image, mask_image, title);

    return;
  }

  void DisplayImage::init_region_label(Image& input_image, Image& mask_image, const Glib::ustring& title)
  {
    int col, row, index;
    int data_value, red_value, green_value, blue_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    if (mask_image.data_valid())
    {
      maskImage = &mask_image; // NOTE: maskImage will initially be all true!
      mask_valid = true;
    }
    else
      mask_valid = false;

    path.clear();

    display_type = 3;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    display_data = new unsigned char[ncols*nrows*3];
    display_highlight = new bool[ncols*nrows];
    temp_highlight = new bool[ncols*nrows];
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        display_highlight[index] = false;
        data_value = (unsigned int) inputImage->get_data(col,row,0);
        red_value = inputImage->get_red_colormap(data_value);
        green_value =inputImage->get_green_colormap(data_value);
        blue_value = inputImage->get_blue_colormap(data_value);
        index = col*3 + row*ncols*3;
        display_data[index] = red_value;
        index++;
        display_data[index] = green_value;
        index++;
        display_data[index] = blue_value;
      }
    }

    set_title(title);

    init();

    return;
  }

  void DisplayImage::reinit_region_label()
  {
    int col, row, index;
    int data_value, red_value, green_value, blue_value;

    path.clear();

    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
#ifdef HIGHLIGHT_COLOR
        if (display_highlight[index])
          data_value = HIGHLIGHT_COLOR;
        else
          data_value = (int) inputImage->get_data(col,row,0);

        red_value = inputImage->get_red_colormap(data_value);
        green_value = inputImage->get_green_colormap(data_value);
        blue_value = inputImage->get_blue_colormap(data_value);
#else
        if (display_highlight[index])
        {
          red_value = 255;
          green_value = 255;
          blue_value = 255;
        }
        else
        {
          data_value = (int) inputImage->get_data(col,row,0);
          red_value = inputImage->get_red_colormap(data_value);
          green_value = inputImage->get_green_colormap(data_value);
          blue_value = inputImage->get_blue_colormap(data_value);
        }
#endif
        index = col*3 + row*ncols*3;
        display_data[index] = red_value;
        index++;
        display_data[index] = green_value;
        index++;
        display_data[index] = blue_value;
      }
    }

    reset_display_thumb_pixbuf();

    thumb_refresh();
    display_refresh();

    return;
  }

  void DisplayImage::init_boundary(Image& input_image, const Glib::ustring& title)
  {
    int  col, row;
    double min_value, max_value, value, scaled_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = NULL;
    mask_valid = false;

    path.clear();

    display_type = 4;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    scale = 1.0;
    offset = 0.0;
    min_value = inputImage->getMinimum(0);
    max_value = inputImage->getMaximum(0);
    if (max_value > min_value)
    {
      scale = 255.0/(min_value - max_value);
      offset = max_value;
    }

    set_display_file();
    displayImage.create(display_file,ncols,nrows,1,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        scaled_value = 0;
        if (inputImage->data_valid(col,row,0))
        {
          value = inputImage->get_data(col,row,0);
          scaled_value = scale*(value-offset);
        }
        displayImage.put_data(scaled_value,col,row,0);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    set_title(title);

    init();

    return;
  }

  void DisplayImage::reinit_boundary()
  {
    int  col, row;
    double min_value, max_value, value, scaled_value;

    path.clear();

    scale = 1.0;
    offset = 0.0;
    inputImage->resetMinMax();
    min_value = inputImage->getMinimum(0);
    max_value = inputImage->getMaximum(0);
    if (max_value > min_value)
    {
      scale = 255.0/(min_value - max_value);
      offset = max_value;
    }

    displayImage.create(display_file,ncols,nrows,1,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        scaled_value = 0;
        if (inputImage->data_valid(col,row,0))
        {
          value = inputImage->get_data(col,row,0);
          scaled_value = scale*(value-offset);
        }
        displayImage.put_data(scaled_value,col,row,0);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    reset_display_thumb_pixbuf();

    return;
  }

  void DisplayImage::init_reference(Image& input_image, const Glib::ustring& title)
  {
    int band;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = NULL;
    mask_valid = false;

    path.clear();

    display_type = 5;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    set_display_file();
    displayImage.create(display_file, ncols, nrows, inputImage->get_nbands(), inputImage->get_data_type(), "BMP");
    for (band = 0; band < inputImage->get_nbands(); band++)
      displayImage.registered_data_copy(band, input_image, band);
    displayImage.flush_data();
    displayImage.close();

    set_title(title);

    init();

    return;
  }

  void DisplayImage::init_float(Image& input_image, const Glib::ustring& title)
  {
    Image mask_image;
    init_float(input_image, mask_image, title);
    return;
  }

  void DisplayImage::init_float(Image& input_image, Image& mask_image, const Glib::ustring& title)
  {
    bool mask_flag;
    int  col, row;
    double min_value, max_value, value, scaled_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = &mask_image;
    mask_valid = mask_image.data_valid();

    path.clear();

    display_type = 6;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);
/*
cout << endl << "Initializing displayimage " << title << endl;
cout << endl << "input_ncols = " << input_ncols << ", and input_nrows = " << input_nrows << endl;
cout << "inputImage_ncols = " << inputImage->get_ncols() << ", and inputImage_nrows = " << inputImage->get_nrows() << endl;
cout << "relative_X_zoom_factor = " << relative_X_zoom_factor << " and relative_Y_zoom_factor = " << relative_Y_zoom_factor << endl;
*/
    bool histo_flag = inputImage->histo_eq_map_valid(0);

    scale = 1.0;
    offset = 0.0;
    if (!histo_flag)
    {
      min_value = inputImage->getMinimum(0,mask_image);
      max_value = inputImage->getMaximum(0,mask_image);
      if (max_value > min_value) 
      {
        scale = 255.0/(max_value - min_value);
        offset = min_value;
      }
    }

    set_display_file();
    displayImage.create(display_file,ncols,nrows,1,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        scaled_value = 0;
        if (inputImage->data_valid(col,row,0))
        {
          mask_flag = true;
          if (mask_valid)
            mask_flag = maskImage->data_valid(col,row,0);
          if (mask_flag)
          {
            value = inputImage->get_data(col,row,0);
            if (histo_flag)
            {
              scaled_value = inputImage->get_histo_lookup(value,0);
            }
            else
            {
              scaled_value = scale*(value-offset);
            } 
          }
        }
        displayImage.put_data(scaled_value,col,row,0);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    set_title(title);

    init();

    return;
  }

  void DisplayImage::reinit_float()
  {
    bool mask_flag;
    int  col, row;
    double value, scaled_value;

    bool histo_flag = inputImage->histo_eq_map_valid(0);

    path.clear();

    displayImage.create(display_file,ncols,nrows,1,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        scaled_value = 0;
        if (inputImage->data_valid(col,row,0))
        {
          mask_flag = true;
          if (mask_valid)
            mask_flag = maskImage->data_valid(col,row,0);
          if (mask_flag)
          {
            value = inputImage->get_data(col,row,0);
            if (histo_flag)
            {
              scaled_value = inputImage->get_histo_lookup(value,0);
            }
            else
            {
              scaled_value = scale*(value-offset);
            } 
          }
        }
        displayImage.put_data(scaled_value,col,row,0);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    reset_display_thumb_pixbuf();

    return;
  }

  void DisplayImage::init_snow(Image& input_image, const Glib::ustring& title)
  {
    int col, row;
    int snow_value, red_value, green_value, blue_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = NULL;
    mask_valid = false;

    path.clear();

    display_type = 7;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    set_display_file();
    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        snow_value = (int) inputImage->get_data(col,row,0);
        switch (snow_value)
        {
          case 0:   red_value = 0; green_value = 0; blue_value = 0; // unlabeled
                    break;
          case 1:   red_value = 255; green_value = 127; blue_value = 0; // no decision
                    break;
          case 11:  red_value = 255; green_value = 0; blue_value = 255; // night
                    break;
          case 25:  red_value = 57; green_value = 162; blue_value = 71; // no snow
                    break;
          case 37:  red_value = 191; green_value = 255; blue_value = 255; // lake
                    break;
          case 39:  red_value = 0; green_value = 0; blue_value = 255; // ocean
                    break;
          case 50:  red_value = 191; green_value = 0; blue_value = 191; // cloud
                    break;
          case 100: red_value = 0; green_value = 255; blue_value = 255; // lake ice
                    break;
          case 200: red_value = 255; green_value = 255; blue_value = 255; // snow
                    break;
          case 254: red_value = 0; green_value = 0; blue_value = 0; // detector saturated
                    break;
          case 255: red_value = 0; green_value = 0; blue_value = 0; // fill
                    break;
          default:  red_value = 0; green_value = 0; blue_value = 0; // (invalid values)
                    break;
        }
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    set_title(title);

    init();

    return;
  }

  void DisplayImage::init_thin_cloud(Image& input_image, const Glib::ustring& title)
  {
    int col, row;
    int thin_cloud_value, red_value, green_value, blue_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = NULL;
    mask_valid = false;

    path.clear();

    display_type = 8;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    set_display_file();
    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        thin_cloud_value = (int) inputImage->get_data(col,row,0);
 // Thin cirrus cloud mask set to 196 if thin cirrus cloud detected by Solar or IR test, and cloud not detected by another cloud test.
 // Thin cirrus cloud mask set to 128 if thin cirrus cloud detected by Solar or IR test, and high cloud detected by another cloud test.
 // Thin cirrus cloud mask set to 96 if thin cirrus cloud not detected by Solar or IR test, and high cloud detected by another cloud test.
 // Thin cirrus cloud mask set to 64 if thin cirrus cloud detected by Solar or IR test, and non-high cloud detected by another cloud test.
 // Thin cirrus cloud mask set ot 32 if thin cirrus cloud not detected, but non-high cloud detected by another cloud test.
 // Set to 0 otherwise, unless the byte 0, bit 0 cloud mask flag is set. In that case it is set to the no_data_value (255).
        switch (thin_cloud_value)
        {
          case 0:   red_value = 0; green_value = 0; blue_value = 0;
                    break;
          case 32:  red_value = 0; green_value = 127; blue_value = 255;
                    break;
          case 64:  red_value = 255; green_value = 255; blue_value = 0;
                    break;
          case 96:  red_value = 255; green_value = 0; blue_value = 255;
                    break;
          case 128: red_value = 0; green_value = 255; blue_value = 255;
                    break;
          case 196: red_value = 255; green_value = 255; blue_value = 255;
                    break;
          case 255: red_value = 0; green_value = 0; blue_value = 0; // no_data_value
                    break;
          default:  red_value = 0; green_value = 0; blue_value = 0; // (invalid values)
                    break;
        }
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    set_title(title);

    init();

    return;
  }

  void DisplayImage::init_revised_snow(Image& input_image, const Glib::ustring& title)
  {
    int col, row, index;
    int snow_value, red_value, green_value, blue_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();
    maskImage = NULL;
    mask_valid = false;

    path.clear();

    display_type = 9;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    set_display_file();
    displayImage.create(display_file,input_image);
    displayImage.registered_data_copy(0,input_image,0);

    display_data = new unsigned char[ncols*nrows*3];
    display_highlight = new bool[ncols*nrows];
    temp_highlight = new bool[ncols*nrows];
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        display_highlight[index] = false;
        snow_value = (int) displayImage.get_data(col,row,0);
        switch (snow_value)
        {
          case 0:   red_value = 0; green_value = 0; blue_value = 0; // unlabeled
                    break;
          case 1:   red_value = 255; green_value = 127; blue_value = 0; // no decision
                    break;
          case 11:  red_value = 255; green_value = 0; blue_value = 255; // night
                    break;
          case 25:  red_value = 57; green_value = 162; blue_value = 71; // no snow
                    break;
          case 37:  red_value = 191; green_value = 255; blue_value = 255; // lake
                    break;
          case 39:  red_value = 0; green_value = 0; blue_value = 255; // ocean
                    break;
          case 50:  red_value = 191; green_value = 0; blue_value = 191; // cloud
                    break;
          case 100: red_value = 0; green_value = 255; blue_value = 255; // lake ice
                    break;
          case 200: red_value = 255; green_value = 255; blue_value = 255; // snow
                    break;
          case 254: red_value = 0; green_value = 0; blue_value = 0; // detector saturated
                    break;
          case 255: red_value = 0; green_value = 0; blue_value = 0; // fill
                    break;
          default:  red_value = 0; green_value = 0; blue_value = 0; // (invalid values)
                    break;
        }
        index = col*3 + row*ncols*3;
        display_data[index] = red_value;
        index++;
        display_data[index] = green_value;
        index++;
        display_data[index] = blue_value;
      }
    }

    set_title(title);

    init();

    return;
  }

  void DisplayImage::reinit_revised_snow()
  {
    int col, row, index;
    int snow_value, red_value, green_value, blue_value;

    path.clear();

    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        if (display_highlight[index])
          snow_value = -1;
        else
          snow_value = (int) displayImage.get_data(col,row,0);
        switch (snow_value)
        {
          case -1:  red_value = 255; green_value = 0; blue_value = 0; // highlighted
                    break;
          case 0:   red_value = 0; green_value = 0; blue_value = 0; // unlabeled
                    break;
          case 1:   red_value = 255; green_value = 127; blue_value = 0; // no decision
                    break;
          case 11:  red_value = 255; green_value = 0; blue_value = 255; // night
                    break;
          case 25:  red_value = 57; green_value = 162; blue_value = 71; // no snow
                    break;
          case 37:  red_value = 191; green_value = 255; blue_value = 255; // lake
                    break;
          case 39:  red_value = 0; green_value = 0; blue_value = 255; // ocean
                    break;
          case 50:  red_value = 191; green_value = 0; blue_value = 191; // cloud
                    break;
          case 100: red_value = 0; green_value = 255; blue_value = 255; // lake ice
                    break;
          case 200: red_value = 255; green_value = 255; blue_value = 255; // snow
                    break;
          case 254: red_value = 0; green_value = 0; blue_value = 0; // detector saturated
                    break;
          case 255: red_value = 0; green_value = 0; blue_value = 0; // fill
                    break;
          default:  red_value = 0; green_value = 0; blue_value = 0; // (invalid values)
                    break;
        }
        index = col*3 + row*ncols*3;
        display_data[index] = red_value;
        index++;
        display_data[index] = green_value;
        index++;
        display_data[index] = blue_value;
      }
    }
    reset_display_thumb_pixbuf();

    thumb_refresh();
    display_refresh();

    return;
  }

  void DisplayImage::init_path(Image& input_image, vector<Point>& input_path, const Glib::ustring& title)
  {
    int col, row;
    unsigned int data_value;
    unsigned char red_value, green_value, blue_value;
    inputImage = &input_image;
    dtype = input_image.get_dtype();

    path.clear();
    add_all(path,input_path);

    display_type = 10;
    zoom_factor = 1.0;
    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    set_display_file();
    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        data_value = (unsigned int) inputImage->get_data(col,row,0);
        red_value = inputImage->get_red_colormap(data_value);
        green_value =inputImage->get_green_colormap(data_value);
        blue_value = inputImage->get_blue_colormap(data_value);
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }

    displayImage.flush_data();
    displayImage.close();

    set_title(title);

    init();

    return;
  }

  void DisplayImage::reinit_path(vector<Point>& input_path)
  {
    int col, row;
    unsigned int data_value;
    unsigned char red_value, green_value, blue_value;

    path.clear();
    add_all(path,input_path);

    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        data_value = (unsigned int) inputImage->get_data(col,row,0);
        red_value = inputImage->get_red_colormap(data_value);
        green_value =inputImage->get_green_colormap(data_value);
        blue_value = inputImage->get_blue_colormap(data_value);
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    reset_display_thumb_pixbuf();

    thumb_refresh();
    display_refresh();

    return;
  }

  void DisplayImage::init_detect(Image& detect_image, Image& site_location_image, const Glib::ustring& title)
  {
    int  col, row;
    double red_value, green_value, blue_value;
    inputImage = &detect_image;
    dtype = detect_image.get_dtype();
    maskImage = &site_location_image;

    path.clear();

    display_type = 11;
    zoom_factor = 1.0;

    ncols = inputImage->get_ncols();
    nrows = inputImage->get_nrows();
    relative_X_zoom_factor = ((double) input_ncols)/((double) ncols);
    relative_Y_zoom_factor = ((double) input_nrows)/((double) nrows);

    set_display_file();
    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        red_value = maskImage->get_data(col,row,0);
        green_value = maskImage->get_data(col,row,1);
        blue_value = maskImage->get_data(col,row,2);
        if (inputImage->get_data(col,row,0) != 0.0)
        {
          if ((red_value == 0) && (green_value == 0) && (blue_value == 0))
          {
            red_value = 255;
            green_value = 255;
            blue_value = 255;
          }
          else
          {
            red_value *= 255;
            green_value *= 255;
            blue_value *= 255;
          }
        }
        else
        {
          red_value *= 127;
          green_value *= 127;
          blue_value *= 127;
        }
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    set_title(title);
    init();

    return;
  }

  void DisplayImage::reinit_detect()
  {
    int  col, row;
    double red_value, green_value, blue_value;

    path.clear();

    displayImage.create(display_file,ncols,nrows,3,GDT_Byte,"BMP");
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        red_value = maskImage->get_data(col,row,0);
        green_value = maskImage->get_data(col,row,1);
        blue_value = maskImage->get_data(col,row,2);
        if (inputImage->get_data(col,row,0) != 0.0)
        {
          if ((red_value == 0) && (green_value == 0) && (blue_value == 0))
          {
            red_value = 255;
            green_value = 255;
            blue_value = 255;
          }
          else
          {
            red_value *= 255;
            green_value *= 255;
            blue_value *= 255;
          }
        }
        else
        {
          red_value *= 127;
          green_value *= 127;
          blue_value *= 127;
        }
        displayImage.put_data(red_value,col,row,0);
        displayImage.put_data(green_value,col,row,1);
        displayImage.put_data(blue_value,col,row,2);
      }
    }
    displayImage.flush_data();
    displayImage.close();

    reset_display_thumb_pixbuf();

    return;
  }

  void DisplayImage::copy_display(Image& copy_image)
  {
    int col,row, index;

    index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        displayImage.put_data(copy_image.get_data(col,row,0),col,row,0);
        display_highlight[index] = false;
        index++;
      }
    displayImage.flush_data();

    return;
  }

  void DisplayImage::revise_display(Image& revise_image)
  {
    int col,row, index;

    index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        if (display_highlight[index])
        {
          displayImage.put_data(revise_image.get_data(col,row,0),col,row,0);
          display_highlight[index] = false;
        }
        index++;
      }
    displayImage.flush_data();

    return;
  }

  void DisplayImage::revise_display(int& label)
  {
    int col,row, index;

    index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        if (display_highlight[index])
        {
          displayImage.put_data(label,col,row,0);
          display_highlight[index] = false;
        }
        index++;
      }
    displayImage.flush_data();

    return;
  }

  void DisplayImage::link_image(DisplayImage *image)
  {

    linkedImages[numberLinked++] = image;

    return;
  }

  void DisplayImage::set_click_location(int& col, int& row)
  {
    colClick = col;
    rowClick = row;
    clickFlag = true;

    return;
  }

  bool DisplayImage::get_click_location(int& col, int& row)
  {
    if (clickFlag)
    {
      col = colClick;
      row = rowClick;
      return true;
    }
    else
      return false;
  }

  void DisplayImage::set_label_mask(const unsigned int& selLabel)
  {
    int row, col;

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
        if (selLabel == (unsigned int) inputImage->get_data(col,row,0))
          maskImage->put_data(true,col,row,0);
        else
          maskImage->put_data(false,col,row,0);
    maskImage->flush_data();

    return;
  }

  bool DisplayImage::set_display_highlight(const unsigned int& selLabel)
  {
    bool mask_flag = true;
    int row, col, index;

    bool label_highlight_flag = false;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        if (mask_valid)
          mask_flag = maskImage->get_data(col,row,0);
        if ((selLabel == (unsigned int) displayImage.get_data(col,row,0)) && mask_flag)
        {
          display_highlight[index] = true;
          label_highlight_flag = true;
        }
        else
        {
          display_highlight[index] = false;
        }
      }

    return label_highlight_flag;
  }

  bool DisplayImage::set_display_highlight(const unsigned int& selLabel, Image& labelImage)
  {
    bool label_highlight_flag = false;
    bool clearFlag = true;

    label_highlight_flag = set_display_highlight(selLabel, labelImage, clearFlag);

    return label_highlight_flag;
  }

  bool DisplayImage::set_display_highlight(const unsigned int& selLabel, Image& labelImage, const bool& clearFlag)
  {
    bool mask_flag = true;
    bool label_highlight_flag = false;
    int row, col, index;

    if ((display_type == 3) || (display_type == 9))
    {
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          index = col + row*ncols;
          if (mask_valid)
            mask_flag = maskImage->get_data(col,row,0);
          if ((selLabel == (unsigned int) labelImage.get_data(col,row,0)) && mask_flag)
          {
            display_highlight[index] = true;
            label_highlight_flag = true;
          }
          else if (clearFlag)
          {
            display_highlight[index] = false;
          }
        }
    }

    return label_highlight_flag;
  }

  bool DisplayImage::set_display_highlight(const unsigned int& selLabel, Image* labelImage)
  {
    bool mask_flag = true;
    bool label_highlight_flag = false;
    int row, col, index;

    if ((display_type == 3) || (display_type == 9))
    {
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          index = col + row*ncols;
          if (mask_valid)
            mask_flag = maskImage->get_data(col,row,0);
          if ((selLabel == (unsigned int) labelImage->get_data(col,row,0)) && mask_flag)
          {
            display_highlight[index] = true;
            label_highlight_flag = true;
          }
          else
          {
            display_highlight[index] = false;
          }
        }
      if (label_highlight_flag)
      {
        if (display_type == 3)
          reinit_region_label();
        else
          reinit_revised_snow();
      }
    }

    return label_highlight_flag;
  }

  bool DisplayImage::set_display_highlight(const float& threshold, float* threshold_array)
  {
    bool thresholdOverUnderFlag = true;

    return set_display_highlight(threshold,threshold_array,thresholdOverUnderFlag);
  }

  bool DisplayImage::set_display_highlight(const float& threshold, float* threshold_array, const bool& thresholdOverUnderFlag)
  {
    bool mask_flag = true;
    int row, col, index;

    bool label_highlight_flag = false;
    if (thresholdOverUnderFlag)
    {
     for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        if (mask_valid)
          mask_flag = maskImage->get_data(col,row,0);
        if ((threshold_array[index] >= threshold) && mask_flag)
        {
          display_highlight[index] = true;
          label_highlight_flag = true;
        }
        else
        {
          display_highlight[index] = false;
        }
      }
    }
    else
    {
     for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        if (mask_valid)
          mask_flag = maskImage->get_data(col,row,0);
        if ((threshold_array[index] < threshold) && mask_flag)
        {
          display_highlight[index] = true;
          label_highlight_flag = true;
        }
        else
        {
          display_highlight[index] = false;
        }
      }
    }

    return label_highlight_flag;
  }

  bool DisplayImage::get_display_highlight(const int& col, const int& row)
  {
    int index;
    index = col + row*ncols;
    return display_highlight[index];
  }

  void DisplayImage::reset_display_highlight()
  {
    int row, col, index;

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        display_highlight[index] = false;
      }

    return;
  }

  void DisplayImage::highlight_region(int& colSelect, int& rowSelect)
  {
    highlight_region(inputImage,colSelect,rowSelect);

    return;
  }
 
  void DisplayImage::highlight_region(Image* sourceImage, int& colSelect, int& rowSelect)
  {
    int col,row, index;

    index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
        display_highlight[index++] = false;
    index = colSelect + rowSelect*ncols;
    display_highlight[index] = true;

  // Do ever expanding search to fill out the selected region 
    int valueSelect = (int) sourceImage->get_data(colSelect,rowSelect,0);
    int minSearchCol = colSelect;
    int maxSearchCol = colSelect;
    int minSearchRow = rowSelect;
    int maxSearchRow = rowSelect;

    int numberOfChanges = 1;
    int totalChanges = 0;

    while (numberOfChanges > 0)
    {
      numberOfChanges = propagate(sourceImage,valueSelect,minSearchCol,maxSearchCol,minSearchRow,maxSearchRow);
      totalChanges += numberOfChanges;
      minSearchCol = minSearchCol - 1;
      if (minSearchCol <= 0) minSearchCol = 1;
        maxSearchCol = maxSearchCol + 1;
      if (maxSearchCol >= (ncols-1))
        maxSearchCol = ncols - 2;
      minSearchRow = minSearchRow - 1;
      if (minSearchRow <= 0)
        minSearchRow = 1;
      maxSearchRow = maxSearchRow + 1;
      if (maxSearchRow >= (nrows-1)) 
        maxSearchRow = nrows - 2;
//      cout << "This iteration, " << numberOfChanges << " changes occurred in propagate function." << endl;
    }

//    cout << "NOTE:  " << totalChanges << " total changes occurred in propagate function.\n" << endl;

    int count = 0;
    index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
        if (display_highlight[index++] == true)
          count++;

    cout << "Selected closed connected regions has " << count << " pixels in it" << endl;

    return;
  }

  void DisplayImage::highlight_select_region(int& colSelect, int& rowSelect)
  {
    int col, row, index;

    index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        temp_highlight[index] = display_highlight[index];
        display_highlight[index++] = false;
      }
    index = colSelect + rowSelect*ncols;
    display_highlight[index] = true;

  // Do ever expanding search to fill out the selected region 
    int minSearchCol = colSelect;
    int maxSearchCol = colSelect;
    int minSearchRow = rowSelect;
    int maxSearchRow = rowSelect;

    int numberOfChanges = 1;
    int totalChanges = 0;

    while (numberOfChanges > 0)
    {
      numberOfChanges = propagate(minSearchCol,maxSearchCol,minSearchRow,maxSearchRow);
      totalChanges += numberOfChanges;
      minSearchCol = minSearchCol - 1;
      if (minSearchCol <= 0) minSearchCol = 1;
        maxSearchCol = maxSearchCol + 1;
      if (maxSearchCol >= (ncols-1))
        maxSearchCol = ncols - 2;
      minSearchRow = minSearchRow - 1;
      if (minSearchRow <= 0)
        minSearchRow = 1;
      maxSearchRow = maxSearchRow + 1;
      if (maxSearchRow >= (nrows-1)) 
        maxSearchRow = nrows - 2;
//      cout << "This iteration, " << numberOfChanges << " changes occurred in propagate function." << endl;
    }

//    cout << "NOTE:  " << totalChanges << " total changes occurred in propagate function.\n" << endl;

    int count = 0;
    index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
        if (display_highlight[index++] == true)
          count++;

    cout << "Selected closed connected regions has " << count << " pixels in it" << endl;

    return;
  }

  void DisplayImage::highlight_circle_roi(Cairo::RefPtr<Cairo::Context>& display_CairoContext)
  {
    int col, row, index;
    double zoom_col, zoom_row;

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        zoom_col = ((double) col)*X_zoom_factor*zoom_factor*relative_X_zoom_factor;
        zoom_row = ((double) row)*Y_zoom_factor*zoom_factor*relative_Y_zoom_factor;
        if ((circle_roi_highlight_all_flag || display_highlight[index]) && display_CairoContext->in_fill(zoom_col,zoom_row))
          display_highlight[index] = true;
        else
          display_highlight[index] = false;
      }

    if (circle_roi_highlight_all_flag)
      m_signal_circle_roi_event.emit();

    return;
  }

  void DisplayImage::clear_path()
  {
    path.clear();

    display_refresh();

    return;
  }

  bool DisplayImage::set_focus(const bool& focusAll)
  {
    if (!clickFlag)
      return false;

    int zoomed_ncols = (int) ((ncols*X_zoom_factor*zoom_factor*relative_X_zoom_factor) + 0.5);
    int zoomed_nrows = (int) ((nrows*Y_zoom_factor*zoom_factor*relative_Y_zoom_factor) + 0.5);

    int zoomed_colClick = (int) (colClick*zoom_factor);
    int zoomed_rowClick = (int) (rowClick*zoom_factor);

    if (focusAll)
    {
      hadjustment_value = zoomed_colClick - visible_ncols/2;
      if ((hadjustment_value + visible_ncols) > zoomed_ncols)
        hadjustment_value = zoomed_ncols - visible_ncols;
      if (hadjustment_value < 0.0)
        hadjustment_value = 0.0;
      hadjustment->set_value(hadjustment_value);
      vadjustment_value = zoomed_rowClick - visible_nrows/2;
      if ((vadjustment_value + visible_nrows) > zoomed_nrows)
        vadjustment_value = zoomed_nrows - visible_nrows;
      if (vadjustment_value < 0.0)
        vadjustment_value = 0.0;
      vadjustment->set_value(vadjustment_value);
    }

    return true;
  }

  bool DisplayImage::set_linked_focus(const bool& focusAll)
  {
    unsigned int index;
    
    if (!clickFlag)
      return false;

    for (index = 0; index < numberLinked; index++)
    {
      linkedImages[index]->set_click_location(colClick,rowClick);
      linkedImages[index]->set_focus(focusAll);
    }

    return true;
  }

  void DisplayImage::display_resize(const int& width, const int& height)
  {
    if ((width != window_ncols) || (height != window_nrows))
    {
      resize(width,height);
      window_ncols = width;
      window_nrows = height;
      visible_nrows = window_nrows - ((int) ((nrows*thumb_factor) + 0.5)) - 100;
      visible_ncols = window_ncols - 42;
      if ((visible_ncols > ncols) && (visible_nrows > nrows))
      {
        visible_ncols = ncols;
        visible_nrows = nrows;
      }
      else if (visible_nrows > nrows)
      {
        visible_nrows = nrows;
        visible_ncols += 16;
      }
      else if (visible_ncols > ncols)
      {
        visible_ncols = ncols;
        visible_nrows += 16;
      }
    }

    resizeFlag = false;

    return;    
  }

  void DisplayImage::remove_display_file()
  {
    if (display_file != "")
      std::remove(display_file.c_str());

    return;
  }

  void DisplayImage::set_button_press_monitor_flag(const bool& value)
  {
    unsigned int index;

    button_press_monitor_flag = value;
    for (index = 0; index < numberLinked; index++)
      linkedImages[index]->button_press_monitor_flag = value;

    return;
  }

  void DisplayImage::on_menu_save_png()
  {
    Gtk::FileChooserDialog dialog("Please specify output file name",
                                  Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK);
    if (current_folder != "")
      dialog.set_current_folder(current_folder);
    if (shortcut_folder != "")
      dialog.add_shortcut_folder(shortcut_folder);

    int result = dialog.run();

    switch(result)
    {
      case (Gtk::RESPONSE_OK):
      {
        string filename = dialog.get_filename();
        cout << "PNG Image to be saved as : " << filename << endl;
        display_Pixbuf->save(dialog.get_filename(),"png");
        break;
      }
      case (Gtk::RESPONSE_CANCEL):
      {
        cout << "Cancel clicked." << endl;
        break;
      }
      default:
      {
        cout << "Unexpected button clicked." << endl;
        break;
      }
    }

    return;
  }

  void DisplayImage::on_menu_save_data()
  {
    Gtk::FileChooserDialog dialog("Please specify output file name",
                                  Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK);
    if (current_folder != "")
      dialog.set_current_folder(current_folder);
    if (shortcut_folder != "")
      dialog.add_shortcut_folder(shortcut_folder);

    int result = dialog.run();

    switch(result)
    {
      case (Gtk::RESPONSE_OK):
      { 
        Image outputImage;
        string filename = dialog.get_filename();
        cout << "Image Data to be saved as : " << filename << endl;
        switch (display_type)
        {
          case 1:  outputImage.create_copy(filename,*inputImage);
                   outputImage.close();
                   break;
          case 2:  outputImage.create_copy(filename,*inputImage);
                   outputImage.close();
                   break;
          case 3:  outputImage.create_copy(filename,*inputImage);
                   outputImage.close();
                   break;
          case 4:  break;
          case 5:  break;
          case 6:  outputImage.create_copy(filename,*inputImage);
                   outputImage.close();
                   break;
          case 7:  break;
          case 8:  break;
          case 9:  outputImage.create_copy(filename,*inputImage);
                   outputImage.close();
                   break;
          case 10: outputImage.create_copy(filename,*inputImage);
                   outputImage.close();
                   break;
          case 11: break;
          default: break;
        }
        break;
      }
      case (Gtk::RESPONSE_CANCEL):
      {
        cout << "Cancel clicked." << endl;
        break;
      }
      default:
      {
        cout << "Unexpected button clicked." << endl;
        break;
      }
    }

    return;
  }

  void DisplayImage::on_menu_zoom_in()
  {
    bool saved_clickFlag = clickFlag;
    int saved_colClick = colClick;
    int saved_rowClick = rowClick;

    clickFlag = true;
    hadjustment_value = hadjustment->get_value();
    colClick = (int) ((hadjustment_value + visible_ncols/2)/(zoom_factor*relative_X_zoom_factor));
    vadjustment_value = vadjustment->get_value();
    rowClick = (int) ((vadjustment_value + visible_nrows/2)/(zoom_factor*relative_Y_zoom_factor));

    double factor = 2.0;

    display_zoom_factor_reset(factor);
    unsigned int index;
    for (index = 0; index < numberLinked; index++)
    {
      linkedImages[index]->display_zoom_factor_reset(factor);
    }

    set_focus(true);
    set_linked_focus(true);
    if (saved_clickFlag)
    {
      colClick = saved_colClick;
      rowClick = saved_rowClick;
    }
    else
      clickFlag = false;

    return;
  }

  void DisplayImage::on_menu_zoom_out()
  {
    bool saved_clickFlag = clickFlag;
    int saved_colClick = colClick;
    int saved_rowClick = rowClick;

    clickFlag = true;
    hadjustment_value = hadjustment->get_value();
    colClick = (int) ((hadjustment_value + visible_ncols/2)/(zoom_factor*relative_X_zoom_factor));
    vadjustment_value = vadjustment->get_value();
    rowClick = (int) ((vadjustment_value + visible_nrows/2)/(zoom_factor*relative_Y_zoom_factor));

    double factor = 0.5;

    display_zoom_factor_reset(factor);
    unsigned int index;
    for (index = 0; index < numberLinked; index++)
    {
      linkedImages[index]->display_zoom_factor_reset(factor);
    }

    set_focus(true);
    set_linked_focus(true);
    if (saved_clickFlag)
    {
      colClick = saved_colClick;
      rowClick = saved_rowClick;
    }
    else
      clickFlag = false;

    return;
  }

  void DisplayImage::on_menu_circle_roi()
  {
    if (hseglearn_flag)
    {
      button_press_monitor_flag = false;
      unsigned int index;
      for (index = 0; index < numberLinked; index++)
      {
        linkedImages[index]->set_button_press_monitor_flag(false);
      }
    }

    circle_roi_flag = true;
    select_region_flag = false; // Added June 17, 2013
    path.clear();
    display_refresh();
    unsigned int index;
    for (index = 0; index < numberLinked; index++)
    {
      linkedImages[index]->display_refresh(path);
    }

    return;
  }

  void DisplayImage::on_menu_select_region()
  {

    if (hseglearn_flag)
    {
      button_press_monitor_flag = true;
      unsigned int index;
      for (index = 0; index < numberLinked; index++)
      {
        linkedImages[index]->set_button_press_monitor_flag(true);
      }
    }
    else
    {
      select_region_flag = true;
cout << "select_region_flag set to true" << endl;
    }

    return;
  }

  void DisplayImage::on_menu_select_off()
  {

    if (hseglearn_flag)
    {
      button_press_monitor_flag = false;
      unsigned int index;
      for (index = 0; index < numberLinked; index++)
      {
        linkedImages[index]->set_button_press_monitor_flag(false);
      }
    }
    else
    {
      select_region_flag = false;
cout << "select_region_flag set to false" << endl;
    }

    return;
  }

  void DisplayImage::on_menu_help()
  {
    cout << "The help menu item was selected." << endl;
    return;
  }

  void DisplayImage::on_menu_file_close()
  {
    hide();
    return;
  }

  bool DisplayImage::on_thumb_button_press_event(GdkEventButton* event)
  {

    unsigned int index;
  // col and row relative to base_ncols and base_nrows
    int col = (int) (event->x/thumb_factor);
    int row = (int) (event->y/thumb_factor);

    if ((col >= 0) && (col < base_ncols) && (row >= 0) && (row < base_nrows))
    {
      colPosition = col;
      rowPosition = row;
      colClick = colPosition;
      rowClick = rowPosition;
      clickFlag = true;

      set_pixel_label();

      for (index = 0; index < numberLinked; index++)
        linkedImages[index]->set_pixel_label(colPosition,rowPosition);

      set_focus(true);
      set_linked_focus(true);

      button_pressed_flag = true;

      return true;
    }

    return false;
  }

  bool DisplayImage::on_thumb_motion_notify_event(GdkEventMotion* event)
  {
    unsigned int index;
  // col and row relative to base_ncols and base_nrows
    int col = (int) (event->x/thumb_factor);
    int row = (int) (event->y/thumb_factor);

    if ((button_pressed_flag) && (col >= 0) && (col < base_ncols) && (row >= 0) && (row < base_nrows))
    {
      colPosition = col;
      rowPosition = row;

      set_pixel_label();

      for (index = 0; index < numberLinked; index++)
        linkedImages[index]->set_pixel_label(colPosition,rowPosition);

      return true;
    }

    return false;
  }

  bool DisplayImage::on_thumb_button_release_event(GdkEventButton* event)
  {
    button_pressed_flag = false;

    return true;
  }

#ifdef GTKMM3
  bool DisplayImage::on_thumb_draw(const Cairo::RefPtr<Cairo::Context>& thumb_CairoContext)
  {
    if (!thumb_Pixbuf)
      return false;
#else
  bool DisplayImage::on_thumb_expose_event(GdkEventExpose* event)
  {
    if (!thumb_Pixbuf)
      return false;

    Glib::RefPtr<Gdk::Window> thumb_Window = thumb_DrawingArea.get_window();
    Cairo::RefPtr<Cairo::Context> thumb_CairoContext = thumb_Window->create_cairo_context();
    if (event)
    {
      thumb_CairoContext->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
      thumb_CairoContext->clip();
    }
#endif
    Gdk::Cairo::set_source_pixbuf(thumb_CairoContext, thumb_Pixbuf, 0, 0);
    thumb_CairoContext->paint();

    return true;
  }

  bool DisplayImage::on_display_button_press_event(GdkEventButton* event)
  {
    unsigned int index;

  // col and row relative to base_ncols and base_nrows
    int col = (int) (event->x/zoom_factor);
    int row = (int) (event->y/zoom_factor);

    if ((col >= 0) && (col < base_ncols) && (row >= 0) && (row < base_nrows))
    {
      colPosition = col;
      rowPosition = row;
      colClick = colPosition;
      rowClick = rowPosition;
      clickFlag = true;

      set_pixel_label();

#ifdef GTKMM3
      Glib::RefPtr <Gdk::Window> ref_Gdk_window = get_window();
      Glib::RefPtr<Gdk::Cursor> display_cursor = Gdk::Cursor::create(ref_Gdk_window->get_display(), Gdk::CROSSHAIR);
#else
      Glib::RefPtr <Gdk::Window> ref_Gdk_window = get_window();
      Gdk::Cursor display_cursor(ref_Gdk_window->get_display(), Gdk::CROSSHAIR);
#endif
      ref_Gdk_window->set_cursor(display_cursor);
      cross_hair_flag = true;
      for (index = 0; index < numberLinked; index++)
        linkedImages[index]->clone_cursor(event->x,event->y);
      set_focus(false);
      set_linked_focus(false);
      if (button_press_monitor_flag)
        m_signal_button_press_event.emit();
      button_pressed_flag = true;

      return true;
    }

    button_pressed_flag = true;

    return false;
  }

  bool DisplayImage::on_display_motion_notify_event(GdkEventMotion* event)
  {
    unsigned int index;

  // col and row relative to base_ncols and base_nrows
    int col = (int) (event->x/zoom_factor);
    int row = (int) (event->y/zoom_factor);

    if (button_pressed_flag)
    {
      if ((col >= 0) && (col < base_ncols) && (row >= 0) && (row < base_nrows))
      {
        colPosition = col;
        rowPosition = row;

        set_pixel_label();

        if (cross_hair_flag)
        {
          for (index = 0; index < numberLinked; index++)
            linkedImages[index]->clone_cursor(event->x,event->y);
        }
        else
        {
#ifdef GTKMM3
          Glib::RefPtr <Gdk::Window> ref_Gdk_window = get_window();
          Glib::RefPtr<Gdk::Cursor> display_cursor = Gdk::Cursor::create(ref_Gdk_window->get_display(), Gdk::CROSSHAIR);
#else
          Glib::RefPtr <Gdk::Window> ref_Gdk_window = get_window();
          Gdk::Cursor display_cursor(ref_Gdk_window->get_display(), Gdk::CROSSHAIR);
#endif
          ref_Gdk_window->set_cursor(display_cursor);
          cross_hair_flag =  true;
          for (index = 0; index < numberLinked; index++)
            linkedImages[index]->clone_cursor(event->x,event->y);
        }

        if (circle_roi_flag)
        {
          Point new_point((int) event->x,(int) event->y);
          path.push_back(new_point);
          display_refresh();
          unsigned int index;
          for (index = 0; index < numberLinked; index++)
          {
            linkedImages[index]->display_refresh(path);
          }
        }

        return true;
      }
      else if (cross_hair_flag)
      {
        Glib::RefPtr <Gdk::Window> ref_Gdk_window = get_window();
        ref_Gdk_window->set_cursor();
        cross_hair_flag = false;
        for (index = 0; index < numberLinked; index++)
          linkedImages[index]->clear_cursor();
      }
    }

    return false;
  }
 
  bool DisplayImage::on_display_button_release_event(GdkEventButton* event)
  {
    unsigned int index;
  // col and row relative to base_ncols and base_nrows
    int col = (int) (event->x/zoom_factor);
    int row = (int) (event->y/zoom_factor);

    if ((col >= 0) && (col < base_ncols) && (row >= 0) && (row < base_nrows))
    {
      colPosition = col;
      rowPosition = row;
      colClick = colPosition;
      rowClick = rowPosition;
      clickFlag = true;

      set_pixel_label();

      set_focus(false);
      set_linked_focus(false);

      if (select_region_flag) // Can only happen for display types 3, 7, 8 and 9.
      {
        int col_select = (int) ((colPosition/X_zoom_factor) + 0.5);
        int row_select = (int) ((rowPosition/Y_zoom_factor) + 0.5);

        select_region_flag = false;

        if ((display_type == 3) || (display_type == 9))
        {

          index = col_select + row_select*ncols;
          if (display_highlight[index])
            highlight_select_region(col_select,row_select);
          else
            highlight_region(col_select,row_select);
            
          if (display_type == 3)
            reinit_region_label();
          else
            reinit_revised_snow();
        }
        else
        {
          int selLabel = (int) inputImage->get_data(col_select,row_select,0);
          for (index = 0; index < numberLinked; index++)
          {
            linkedImages[index]->set_display_highlight(selLabel,inputImage);
          }
        }
      }
      else if (circle_roi_flag)
      {
        button_pressed_flag = false;
        circle_roi_flag = false;
        Point new_point((int) path[0].get_x(),(int) path[0].get_y());
        path.push_back(new_point);
        display_refresh();
        unsigned int index;
        for (index = 0; index < numberLinked; index++)
        {
          linkedImages[index]->display_refresh(path);
        }
      }
    }

    if (cross_hair_flag)
    {
      Glib::RefPtr <Gdk::Window> ref_Gdk_window = get_window();
      ref_Gdk_window->set_cursor();
      cross_hair_flag = false;
      for (index = 0; index < numberLinked; index++)
        linkedImages[index]->clear_cursor();
    }

    button_pressed_flag = false;

    return true;
  }

#ifdef GTKMM3
  bool DisplayImage::on_display_draw(const Cairo::RefPtr<Cairo::Context>& display_CairoContext)
  {
    if (!display_Pixbuf)
      return false;
#else 
  bool DisplayImage::on_display_expose_event(GdkEventExpose* event)
  {
    if (!display_Pixbuf)
      return false;

    Glib::RefPtr<Gdk::Window> display_Window = display_DrawingArea.get_window();
    Cairo::RefPtr<Cairo::Context> display_CairoContext = display_Window->create_cairo_context();
    if (event)
    {
      display_CairoContext->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
      display_CairoContext->clip();
    }
#endif
    Gdk::Cairo::set_source_pixbuf(display_CairoContext,display_Pixbuf,0,0);
    display_CairoContext->paint();

    if (path.size() > 0)
    {
      draw_path(display_CairoContext);
      display_CairoContext->stroke();
    }

    return false;
  }

  void DisplayImage::on_hadjustment_value_changed()
  {
    unsigned int index;

    hadjustment_value = hadjustment->get_value();
    for (index = 0; index < numberLinked; index++)
      linkedImages[index]->hadjustment->set_value(hadjustment_value);

    return;
  }

  void DisplayImage::on_vadjustment_value_changed()
  {
    unsigned int index;

    vadjustment_value = vadjustment->get_value();
    for (index = 0; index < numberLinked; index++)
      linkedImages[index]->vadjustment->set_value(vadjustment_value);

    return;
  }

  void DisplayImage::on_window_check_resize()
  {
    int width, height;

    get_size(width,height);

    if ((width != window_ncols) || (height != window_nrows))
    {
      window_ncols = width;
      window_nrows = height;
      visible_nrows = window_nrows - ((int) ((nrows*thumb_factor) + 0.5)) - 100;
      visible_ncols = window_ncols - 42;
      if ((visible_ncols > ncols) && (visible_nrows > nrows))
      {
        visible_ncols = ncols;
        visible_nrows = nrows;
      }
      else if (visible_nrows > nrows)
      {
        visible_nrows = nrows;
        visible_ncols += 16;
      }
      else if (visible_ncols > ncols)
      {
        visible_ncols = ncols;
        visible_nrows += 16;
      }
      resizeFlag = true;
    }

    return;
  }

  bool DisplayImage::on_window_enter_notify_event(GdkEventCrossing* event)
  {
    if (resizeFlag)
    {
      int width, height;
      unsigned int index;

      get_size(width,height);

      for (index = 0; index < numberLinked; index++)
        linkedImages[index]->display_resize(width,height);

      resizeFlag = false;
    }

    return true;
  }

  bool DisplayImage::on_window_leave_notify_event(GdkEventCrossing* event)
  {
    if (resizeFlag)
    {
      int width, height;
      unsigned int index;

      get_size(width,height);

      for (index = 0; index < numberLinked; index++)
        linkedImages[index]->display_resize(width,height);

      resizeFlag = false;
    }

    return true;
  }

  DisplayImage::type_signal_button_press_event DisplayImage::signal_button_press_event()
  {
    return m_signal_button_press_event;
  }

  DisplayImage::type_signal_circle_roi_event DisplayImage::signal_circle_roi_event()
  {
    return m_signal_circle_roi_event;
  }

  bool DisplayImage::set_display_file()
  {

   // Set temporary file name (including path)
    const char *temp_dir;
    temp_dir = getenv("TMP");
    if (temp_dir == NULL)
      temp_dir = getenv("TEMP");
    if (temp_dir == NULL)
      temp_dir = getenv("TMPDIR");
    if (temp_dir == NULL)
    {
      temp_dir = (char *) malloc(5*sizeof(char));
      string tmp = "/tmp";
      temp_dir = tmp.c_str();
    }
    string temp_directory = temp_dir;

    static char time_buffer[TIME_SIZE];
    time_t now;
    const  struct tm *tm_ptr;
    now = time(NULL);
    tm_ptr = localtime(&now);
/*
    int length;
    length = strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);
*/
    strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);

    string temp_file_name = time_buffer;
#ifdef MSYS
    string slash = "\\";
#else
    string slash = "/";
#endif

    ifstream io_file_fs;
    int instance = 0;
    ostringstream instance_string;
    instance_string << instance;
    switch (display_type)
    {
        case 1:  display_file = temp_directory + slash + "DIRGB" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIRGB" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 2:  display_file = temp_directory + slash + "DISeg" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DISeg" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 3:  display_file = temp_directory + slash + "DIRCur" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIRCur" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 4:  display_file = temp_directory + slash + "DIHBM" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIHBM" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 5:  display_file = temp_directory + slash + "DIRef" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIRef" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 6:  display_file = temp_directory + slash + "DIFLT" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIFLT" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 7:  display_file = temp_directory + slash + "DISnow" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DISnow" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 8:  display_file = temp_directory + slash + "DIThinCloud" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIThinCloud" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 9:  display_file = temp_directory + slash + "DIRevSnow" + instance_string.str() + temp_file_name + ".tif";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIRevSnow" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 10: display_file = temp_directory + slash + "DIPath" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIPath" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        case 11: display_file = temp_directory + slash + "DIDetect" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIDetect" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
        default: display_file = temp_directory + slash + "DIUnk" + instance_string.str() + temp_file_name + ".bmp";
                 io_file_fs.open(display_file.c_str( ));
                 while (io_file_fs)
                 {
                  // File already exists - display_file until a unique file name is found
                   io_file_fs.close( );
                   instance++;
                   instance_string << instance;
                   display_file = temp_directory + slash + "DIUnk" + instance_string.str() + temp_file_name + ".bmp";
                   io_file_fs.open(display_file.c_str( ));
                 }
                 break;
    }

    return true;
  }

  void DisplayImage::calc_defaults()
  {
    visible_ncols = MAX_VISIBLE_SIZE;
    visible_nrows = MAX_VISIBLE_SIZE;

    if (visible_ncols > base_ncols)
      visible_ncols = base_ncols;
    if (visible_nrows > base_nrows)
      visible_nrows = base_nrows;

    thumb_factor = ((double) MAX_VISIBLE_SIZE)/((double) base_ncols);
    if (((double) MAX_THUMB_SIZE)/((double) base_nrows) < thumb_factor)
      thumb_factor = ((double) MAX_THUMB_SIZE)/((double) base_nrows);
    if (thumb_factor > 0.5)
      thumb_factor = 0.5;

    window_ncols = visible_ncols + 42;
    if (visible_ncols < MAX_VISIBLE_SIZE)
      window_ncols -= 16;
    window_nrows = ((int) (base_nrows*thumb_factor + 0.5)) + visible_nrows + 100;
    if (visible_nrows < MAX_VISIBLE_SIZE)
      window_nrows -= 16;

    return;
  }

  double DisplayImage::offset_for_rounding(const double& value)
  {
     if (value >= -0.5)
       return (value + 0.5);
     else
       return (value - 0.5);
  }

  void DisplayImage::init()
  {

    calc_defaults();
    set_default_size(window_ncols,window_nrows);

    add(m_Box); // put a MenuBar at the top of the box and other stuff below it.

    //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();
 
    //Action menu:
    m_refActionGroup->add(Gtk::Action::create("ActionMenu", "Actions"));

    //Help Sub-menu:
    m_refActionGroup->add( Gtk::Action::create("Help", Gtk::Stock::HELP),
            sigc::mem_fun(*this, &DisplayImage::on_menu_help) );
    m_refActionGroup->add(Gtk::Action::create("Save PNG", "Save PNG Image"),
            sigc::mem_fun(*this, &DisplayImage::on_menu_save_png));

    if ((display_type == 2) || (display_type == 3) || (display_type == 9))
      m_refActionGroup->add(Gtk::Action::create("Save Data", "Save Image Data"),
              sigc::mem_fun(*this, &DisplayImage::on_menu_save_data));

    m_refActionGroup->add(Gtk::Action::create("Zoom In", "Zoom In"),
                          Gtk::AccelKey("<control>i"),
                          sigc::mem_fun(*this, &DisplayImage::on_menu_zoom_in));

    m_refActionGroup->add(Gtk::Action::create("Zoom Out", "Zoom Out"),
                          Gtk::AccelKey("<control>o"),
                          sigc::mem_fun(*this, &DisplayImage::on_menu_zoom_out));

    if (hseglearn_flag)
    {
      m_refActionGroup->add(Gtk::Action::create("Circling", "Highlight by Circling an Area of Interest"),
                            Gtk::AccelKey("<control>a"),
                            sigc::mem_fun(*this, &DisplayImage::on_menu_circle_roi));
      m_refActionGroup->add(Gtk::Action::create("Clicking", "Highlight by Clicking on a Region Object"),
                            Gtk::AccelKey("<control>s"),
                            sigc::mem_fun(*this, &DisplayImage::on_menu_select_region));
      m_refActionGroup->add(Gtk::Action::create("Clicking Off", "Turn off Highlight by Clicking"),
                            Gtk::AccelKey("<control>x"),
                            sigc::mem_fun(*this, &DisplayImage::on_menu_select_off));

    }
    else
    {
      if ((display_type == 3) || (display_type == 7) || (display_type == 8) || (display_type == 9))
        m_refActionGroup->add(Gtk::Action::create("Select Region", "Select Region"),
                              Gtk::AccelKey("<control>s"),
                              sigc::mem_fun(*this, &DisplayImage::on_menu_select_region));

      if ((display_type == 1) || (display_type == 2) || (display_type == 3) || (display_type == 5) ||
          (display_type == 6) || (display_type == 7) || (display_type == 8) || (display_type == 9))
        m_refActionGroup->add(Gtk::Action::create("Circle ROI", "Circle Region of Interest"),
                              Gtk::AccelKey("<control>r"),
                              sigc::mem_fun(*this, &DisplayImage::on_menu_circle_roi));
    }

    m_refActionGroup->add(Gtk::Action::create("Close", Gtk::Stock::CLOSE),
            sigc::mem_fun(*this, &DisplayImage::on_menu_file_close));

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);

    add_accel_group(m_refUIManager->get_accel_group());

    //Layout the actions in a menubar:
    Glib::ustring ui_info;
    if (hseglearn_flag)
    {
      if ((display_type == 1) || (display_type == 5) || (display_type == 6))
      {
        ui_info = 
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='ActionMenu'>"
            "      <menuitem action='Help'/>"
            "      <separator/>"
            "      <menuitem action='Save PNG'/>"
            "      <separator/>"
            "      <menuitem action='Zoom In'/>"
            "      <menuitem action='Zoom Out'/>"
            "      <separator/>"
            "      <menuitem action='Circling'/>"
            "      <menuitem action='Clicking'/>"
            "      <menuitem action='Clicking Off'/>"
            "      <separator/>"
            "      <menuitem action='Close'/>"
            "    </menu>"
            "  </menubar>"
            "</ui>";
      }
      else // if (display_type == 3)
      {
        ui_info = 
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='ActionMenu'>"
            "      <menuitem action='Help'/>"
            "      <separator/>"
            "      <menuitem action='Save PNG'/>"
            "      <separator/>"
            "      <menuitem action='Save Data'/>"
            "      <separator/>"
            "      <menuitem action='Zoom In'/>"
            "      <menuitem action='Zoom Out'/>"
            "      <separator/>"
            "      <menuitem action='Circling'/>"
            "      <menuitem action='Clicking'/>"
            "      <menuitem action='Clicking Off'/>"
            "      <separator/>"
            "      <menuitem action='Close'/>"
            "    </menu>"
            "  </menubar>"
            "</ui>";
      } 
    }
    else if ((display_type == 1) || (display_type == 5) || (display_type == 6))
    {
      ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Save PNG'/>"
          "      <separator/>"
          "      <menuitem action='Zoom In'/>"
          "      <menuitem action='Zoom Out'/>"
          "      <separator/>"
          "      <menuitem action='Circle ROI'/>"
          "      <separator/>"
          "      <menuitem action='Close'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";
    }
    else if (display_type == 2)
    {
      ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Save PNG'/>"
          "      <separator/>"
          "      <menuitem action='Save Data'/>"
          "      <separator/>"
          "      <menuitem action='Zoom In'/>"
          "      <menuitem action='Zoom Out'/>"
          "      <separator/>"
          "      <menuitem action='Circle ROI'/>"
          "      <separator/>"
          "      <menuitem action='Close'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";
    }
    else if ((display_type == 7) || (display_type == 8))
    {
      ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Save PNG'/>"
          "      <separator/>"
          "      <menuitem action='Zoom In'/>"
          "      <menuitem action='Zoom Out'/>"
          "      <separator/>"
          "      <menuitem action='Select Region'/>"
          "      <menuitem action='Circle ROI'/>"
          "      <separator/>"
          "      <menuitem action='Close'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";
    }
    else if ((display_type == 3) || (display_type == 9))
    {
      ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Save PNG'/>"
          "      <separator/>"
          "      <menuitem action='Save Data'/>"
          "      <separator/>"
          "      <menuitem action='Zoom In'/>"
          "      <menuitem action='Zoom Out'/>"
          "      <separator/>"
          "      <menuitem action='Select Region'/>"
          "      <menuitem action='Circle ROI'/>"
          "      <separator/>"
          "      <menuitem action='Close'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";
    } 
    else
    {
      ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Save PNG'/>"
          "      <separator/>"
          "      <menuitem action='Zoom In'/>"
          "      <menuitem action='Zoom Out'/>"
          "      <separator/>"
          "      <menuitem action='Close'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";
    }
                                                                                                           
#if (defined(GLIBMM_EXCEPTIONS_ENABLED) || (defined(ARTEMIS)))
    try
    {
      m_refUIManager->add_ui_from_string(ui_info);
    }
    catch(const Glib::Error& ex)
    {
      cerr << "building menus failed: " <<  ex.what();
    }
#else
    auto_ptr<Glib::Error> ex;
    m_refUIManager->add_ui_from_string(ui_info, ex);
    if(ex.get())
    {
      cerr << "building menus failed: " <<  ex->what();
    }
#endif //GLIBMM_EXCEPTIONS_ENABLED

    //Get the menubar widget, and add it to a container widget:
    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    if(pMenubar)
      m_Box.pack_start(*pMenubar, Gtk::PACK_SHRINK);

  // Complete initialization of widgets
    set_display_thumb_pixbuf();

    display_DrawingArea.set_size_request(display_ncols,display_nrows);
    display_DrawingArea.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    thumb_DrawingArea.set_size_request(thumb_Pixbuf->get_width(), thumb_Pixbuf->get_height());
    thumb_DrawingArea.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    display_ScrolledWindow.set_border_width(10);
    display_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    hadjustment = display_ScrolledWindow.get_hadjustment();
    vadjustment = display_ScrolledWindow.get_vadjustment();
    display_ScrolledWindow.add(display_DrawingArea);

    pixel_Label.set_text("Click on image to show (col,row)");
    pixel_Label2.set_text(" ");

    thumb_DrawingArea.signal_button_press_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_thumb_button_press_event));
    thumb_DrawingArea.signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_thumb_motion_notify_event));
    thumb_DrawingArea.signal_button_release_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_thumb_button_release_event));
#ifdef GTKMM3
    thumb_DrawingArea.signal_draw().connect(
        sigc::mem_fun(*this, &DisplayImage::on_thumb_draw));
#else
    thumb_DrawingArea.signal_expose_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_thumb_expose_event));
#endif

    display_DrawingArea.signal_button_press_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_display_button_press_event));
    display_DrawingArea.signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_display_motion_notify_event));
    display_DrawingArea.signal_button_release_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_display_button_release_event));
#ifdef GTKMM3
    display_DrawingArea.signal_draw().connect(
        sigc::mem_fun(*this, &DisplayImage::on_display_draw));
#else
    display_DrawingArea.signal_expose_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_display_expose_event));
#endif
    hadjustment->signal_value_changed().connect(
        sigc::mem_fun(*this, &DisplayImage::on_hadjustment_value_changed));
    vadjustment->signal_value_changed().connect(
        sigc::mem_fun(*this, &DisplayImage::on_vadjustment_value_changed));
    signal_check_resize().connect(
        sigc::mem_fun(*this, &DisplayImage::on_window_check_resize));
    signal_enter_notify_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_window_enter_notify_event));
    signal_leave_notify_event().connect(
        sigc::mem_fun(*this, &DisplayImage::on_window_leave_notify_event));

  // Pack the GUI panel
    m_Box.pack_start(thumb_DrawingArea, Gtk::PACK_SHRINK);
    m_Box.pack_start(display_ScrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    m_Box.pack_start(pixel_Label, Gtk::PACK_SHRINK);

    if (geotransform_flag)
      m_Box.pack_start(pixel_Label2, Gtk::PACK_SHRINK);

    show_all_children();

    return;
  }
  
  void DisplayImage::set_display_thumb_pixbuf()
  {
/*
cout << endl << "Entering set_display_thumb_pixbuf for display_type " << (int) display_type;
cout << ", ncols = " << ncols << " and nrows = " << nrows << endl;
cout << "X_zoom_factor = " << X_zoom_factor << " and Y_zoom_factor = " << Y_zoom_factor << endl;
cout << "zoom_factor = " << zoom_factor << ", relative_X_zoom_factor = " << relative_X_zoom_factor;
cout << " and relative_Y_zoom_factor = " << relative_Y_zoom_factor << endl;
*/
    display_ncols = (int) ((ncols*X_zoom_factor*zoom_factor*relative_X_zoom_factor) + 0.5);
    display_nrows = (int) ((nrows*Y_zoom_factor*zoom_factor*relative_Y_zoom_factor) + 0.5);

    thumb_ncols = (int) (display_ncols*thumb_factor + 0.5);
    thumb_nrows = (int) (display_nrows*thumb_factor + 0.5);

    if ((display_type == 3) || (display_type == 9))
      full_Pixbuf = Gdk::Pixbuf::create_from_data(display_data,Gdk::COLORSPACE_RGB,
                                                 false,8,ncols,nrows,3*ncols);
    else
      full_Pixbuf = Gdk::Pixbuf::create_from_file(display_file);

    thumb_Pixbuf = full_Pixbuf->scale_simple(thumb_ncols,thumb_nrows,Gdk::INTERP_NEAREST);
    thumb_DrawingArea.queue_draw();

    display_Pixbuf = full_Pixbuf->scale_simple(display_ncols,display_nrows,Gdk::INTERP_NEAREST);
    display_DrawingArea.queue_draw();

    return;
  }

  void DisplayImage::reset_display_thumb_pixbuf()
  {
    if ((display_type == 3) || (display_type == 9))
      full_Pixbuf = Gdk::Pixbuf::create_from_data(display_data,Gdk::COLORSPACE_RGB,
                                                 false,8,ncols,nrows,3*ncols);
    else
      full_Pixbuf = Gdk::Pixbuf::create_from_file(display_file);
    thumb_Pixbuf = full_Pixbuf->scale_simple(thumb_ncols,thumb_nrows,Gdk::INTERP_NEAREST);
    thumb_DrawingArea.queue_draw();

    display_Pixbuf = full_Pixbuf->scale_simple(display_ncols,display_nrows,Gdk::INTERP_NEAREST);
    display_DrawingArea.queue_draw();

    return;
  }

  void DisplayImage::display_zoom_factor_reset(const double& factor)
  {
    zoom_factor *= factor;
    display_ncols = (int) ((ncols*zoom_factor*relative_X_zoom_factor*X_zoom_factor) + 0.5);
    display_nrows = (int) ((nrows*zoom_factor*relative_Y_zoom_factor*Y_zoom_factor) + 0.5);
    display_Pixbuf = full_Pixbuf->scale_simple(display_ncols,display_nrows,Gdk::INTERP_NEAREST);
    display_DrawingArea.set_size_request(display_ncols,display_nrows);

    return;
  }

  void DisplayImage::set_pixel_label()
  {
    char label[80];
    int col = (int) ((colPosition/(X_zoom_factor*relative_X_zoom_factor)) + 0.5);
    int row = (int) ((rowPosition/(Y_zoom_factor*relative_Y_zoom_factor)) + 0.5);
    float red_value, green_value, blue_value, grey_value;
    int snow_value, thin_cloud_value;
/*
cout << endl << "For display_type = " << (int) display_type << ", colPosition = " << colPosition << " and rowPosition = " << rowPosition << endl;
cout << "zoom_factor = " << zoom_factor << ", relative_X_zoom_factor = " << relative_X_zoom_factor;
cout << " and relative_Y_zoom_factor = " << relative_Y_zoom_factor << endl;
cout << "col = " << col << " and row = " << row << endl;
cout << "ncols = " << ncols << " and nrows = " << nrows << endl;
*/
    switch (display_type)
    {
      case 1:  red_value = inputImage->get_data(col,row,inputImage->get_red_display_band());
               green_value = inputImage->get_data(col,row,inputImage->get_green_display_band());
               blue_value = inputImage->get_data(col,row,inputImage->get_blue_display_band());
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : (R,G,B) = (%f,%f,%f)",
                         col,row,red_value,green_value,blue_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : (R,G,B) = (%d,%d,%d)",
                         col,row,(int) red_value,(int) green_value,(int) blue_value);
               break;
      case 2:  grey_value = inputImage->get_data(col,row,0);
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : value = %f",
                         col,row,grey_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : value = %d",
                         col,row,(int) grey_value);
               break;
      case 3:  grey_value = inputImage->get_data(col,row,0);
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : value = %f",
                         col,row,grey_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : value = %d",
                         col,row,(int) grey_value);
               break;
      case 4:  grey_value = inputImage->get_data(col,row,0);
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : value = %f",
                         col,row,grey_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : value = %d",
                         col,row,(int) grey_value);
               break;
      case 5:  red_value = inputImage->get_data(col,row,0);
               if (inputImage->get_nbands() > 1)
                 green_value = inputImage->get_data(col,row,1);
               else
                 green_value = red_value;
               if (inputImage->get_nbands() > 2)
                 blue_value = inputImage->get_data(col,row,2);
               else
                 blue_value = green_value;
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : (R,G,B) = (%f,%f,%f)",
                         col,row,red_value,green_value,blue_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : (R,G,B) = (%d,%d,%d)",
                         col,row,(int) red_value,(int) green_value,(int) blue_value);
               break;
      case 6:  grey_value = inputImage->get_data(col,row,0);
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : value = %f",
                         col,row,grey_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : value = %d",
                         col,row,(int) grey_value);
               break;
      case 7:  snow_value = (int) inputImage->get_data(col,row,0);
               switch (snow_value)
               {
                 case 0:   sprintf(label,"(col,row) = (%d,%d) : missing data (0)",col,row);
                           break;
                 case 1:   sprintf(label,"(col,row) = (%d,%d) : no decision (1)",col,row);
                           break;
                 case 11:  sprintf(label,"(col,row) = (%d,%d) : night (11)",col,row);
                           break;
                 case 25:  sprintf(label,"(col,row) = (%d,%d) : no snow (25)",col,row);
                           break;
                 case 37:  sprintf(label,"(col,row) = (%d,%d) : lake (37)",col,row);
                           break;
                 case 39:  sprintf(label,"(col,row) = (%d,%d) : ocean (39)",col,row);
                           break;
                 case 50:  sprintf(label,"(col,row) = (%d,%d) : cloud (50)",col,row);
                           break;
                 case 100: sprintf(label,"(col,row) = (%d,%d) : lake ice (100)",col,row);
                           break;
                 case 200: sprintf(label,"(col,row) = (%d,%d) : snow (200)",col,row);
                           break;
                 case 254: sprintf(label,"(col,row) = (%d,%d) : detector saturated (254)",col,row);
                           break;
                 case 255: sprintf(label,"(col,row) = (%d,%d) : fill (255)",col,row);
                           break;
                 default:  sprintf(label,"(col,row) = (%d,%d)",col,row);
                           break;
               }
               break;
      case 8:  thin_cloud_value = (int) inputImage->get_data(col,row,0);
               switch (thin_cloud_value)
               {
                 case 0:   sprintf(label,"(col,row) = (%d,%d) : no cloud (0)",col,row);
                           break;
                 case 32:  sprintf(label,"(col,row) = (%d,%d) : cloud (32)",col,row);
                           break;
                 case 64:  sprintf(label,"(col,row) = (%d,%d) : thin cirrus and cloud (64)",col,row);
                           break;
                 case 96:  sprintf(label,"(col,row) = (%d,%d) : high cloud (96)",col,row);
                           break;
                 case 128: sprintf(label,"(col,row) = (%d,%d) : thin cirrus and high cloud (128)",col,row);
                           break;
                 case 196: sprintf(label,"(col,row) = (%d,%d) : thin cirrus (196)",col,row);
                           break;
                 case 255: sprintf(label,"(col,row) = (%d,%d) : no data (255)",col,row);
                           break;
                 default:  sprintf(label,"(col,row) = (%d,%d)",col,row);
                           break;
               }
               break;
      case 9:  snow_value = (int) displayImage.get_data(col,row,0);
               switch (snow_value)
               {
                 case 0:   sprintf(label,"(col,row) = (%d,%d) : missing data (0)",col,row);
                           break;
                 case 1:   sprintf(label,"(col,row) = (%d,%d) : no decision (1)",col,row);
                           break;
                 case 11:  sprintf(label,"(col,row) = (%d,%d) : night (11)",col,row);
                           break;
                 case 25:  sprintf(label,"(col,row) = (%d,%d) : no snow (25)",col,row);
                           break;
                 case 37:  sprintf(label,"(col,row) = (%d,%d) : lake (37)",col,row);
                           break;
                 case 39:  sprintf(label,"(col,row) = (%d,%d) : ocean (39)",col,row);
                           break;
                 case 50:  sprintf(label,"(col,row) = (%d,%d) : cloud (50)",col,row);
                           break;
                 case 100: sprintf(label,"(col,row) = (%d,%d) : lake ice (100)",col,row);
                           break;
                 case 200: sprintf(label,"(col,row) = (%d,%d) : snow (200)",col,row);
                           break;
                 case 254: sprintf(label,"(col,row) = (%d,%d) : detector saturated (254)",col,row);
                           break;
                 case 255: sprintf(label,"(col,row) = (%d,%d) : fill (255)",col,row);
                           break;
                 default:  sprintf(label,"(col,row) = (%d,%d)",col,row);
                           break;
               }
               break;
      case 10: grey_value = inputImage->get_data(col,row,0);
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : value = %f",
                         col,row,grey_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : value = %d",
                         col,row,(int) grey_value);
               break;
      case 11: grey_value = inputImage->get_data(col,row,0);
               if (dtype == Float32)
                 sprintf(label,"(col,row) = (%d,%d) : value = %f",
                         col,row,grey_value);
               else
                 sprintf(label,"(col,row) = (%d,%d) : value = %d",
                         col,row,(int) grey_value);
               break;
      default: sprintf(label,"(col,row) = (%d,%d)",col,row);
               break;
    }
    pixel_Label.set_text(label);

    if (geotransform_flag)
    {
      double UTM_X, UTM_Y;
      UTM_X = X_offset + col*base_X_gsd;
      UTM_Y = Y_offset + row*base_Y_gsd;
      sprintf(label,"(UTM_X,UTM_Y) = (%f,%f)",UTM_X,UTM_Y);
      pixel_Label2.set_text(label);
    }

    return;
  }

  void DisplayImage::set_pixel_label(const int& colPos, const int& rowPos)
  {
    colPosition = colPos;
    rowPosition = rowPos;
    set_pixel_label();

    return;
  }

  void DisplayImage::clone_cursor(gdouble& x, gdouble& y)
  {
    int col = (int) x;
    int row = (int) y;

    colPosition = col;
    rowPosition = row;

    set_pixel_label();
  // Draw a floating crosshair at the (x,y) location.
    Glib::RefPtr<Gdk::Window> display_Window = display_DrawingArea.get_window();
    if (display_Window)
    {
      Cairo::RefPtr<Cairo::Context> display_CairoContext = display_Window->create_cairo_context();
      Gdk::Cairo::set_source_pixbuf(display_CairoContext, display_Pixbuf, 0, 0);
      display_CairoContext->paint();
      display_CairoContext->set_line_width(1.0);
      display_CairoContext->set_source_rgb(1.0,1.0,0.0);
      display_CairoContext->move_to(x-8,y);
      display_CairoContext->line_to(x+8,y);
      display_CairoContext->move_to(x,y-8);
      display_CairoContext->line_to(x,y+8);
      display_CairoContext->stroke();
      display_CairoContext->set_source_rgb(1.0,1.0,1.0);
      display_CairoContext->move_to(x-4,y);
      display_CairoContext->line_to(x+4,y);
      display_CairoContext->move_to(x,y-4);
      display_CairoContext->line_to(x,y+4);
      display_CairoContext->stroke();
    }

    return;
  }

  void DisplayImage::clear_cursor()
  {
    Glib::RefPtr<Gdk::Window> display_Window = display_DrawingArea.get_window();
    if (display_Window)
    {
      Cairo::RefPtr<Cairo::Context> display_CairoContext = display_Window->create_cairo_context();
      Gdk::Cairo::set_source_pixbuf(display_CairoContext, display_Pixbuf, 0, 0);
      display_CairoContext->paint();
    }

    return;
  }

  void DisplayImage::thumb_refresh()
  {
    Glib::RefPtr<Gdk::Window> thumb_Window = thumb_DrawingArea.get_window();

    if (thumb_Window)
    {
      Cairo::RefPtr<Cairo::Context> thumb_CairoContext = thumb_Window->create_cairo_context();
      Gdk::Cairo::set_source_pixbuf(thumb_CairoContext,thumb_Pixbuf,0,0);
      thumb_CairoContext->paint();
    }

    return;
  }

  void DisplayImage::display_refresh(vector<Point>& input_path)
  {
    path.clear();
    add_all(path,input_path);

    display_refresh();

    return;
  }

  void DisplayImage::display_refresh()
  {
    Glib::RefPtr<Gdk::Window> display_Window = display_DrawingArea.get_window();

    if (display_Window)
    {
      Cairo::RefPtr<Cairo::Context> display_CairoContext = display_Window->create_cairo_context();
      Gdk::Cairo::set_source_pixbuf(display_CairoContext,display_Pixbuf,0,0);
      display_CairoContext->paint();

      if (path.size() > 0)
      {
        draw_path(display_CairoContext);
        if ((display_type == 3) || (display_type == 9))
        {
          if ((path.size() > 1) && (path[0] == path[path.size()-1]))
          {
            display_CairoContext->stroke_preserve();
            if (display_type == 3)
            {
              highlight_circle_roi(display_CairoContext);
              reinit_region_label();
            }
            if (display_type == 9)
            {
              highlight_circle_roi(display_CairoContext);
              reinit_revised_snow();
            }
          }
          else
            display_CairoContext->stroke();
        }
        else
          display_CairoContext->stroke();
      }
    }

    return;
  }

  void DisplayImage::draw_path(const Cairo::RefPtr<Cairo::Context>& display_CairoContext)
  {
    display_CairoContext->set_line_width(2.0);
    display_CairoContext->set_source_rgb(1.0,1.0,1.0);

    vector<Point>::const_iterator path_iter = path.begin();
    display_CairoContext->move_to((*path_iter).get_col(),(*path_iter).get_row());
    if (display_type == 10)
    {
      display_CairoContext->set_font_size(20.0);
      display_CairoContext->show_text("S");
      display_CairoContext->move_to((*path_iter).get_col(),(*path_iter).get_row());
    }
    ++path_iter;
    while (path_iter != path.end())
    {
      display_CairoContext->line_to((*path_iter).get_col(),(*path_iter).get_row());
      ++path_iter;
    }
    if (display_type == 10)
    {
      display_CairoContext->show_text("E");
    }

    return;
  }

  int DisplayImage::propagate(Image* sourceImage, int searchValue, 
                              int minSearchCol, int maxSearchCol, int minSearchRow, int maxSearchRow)
  {
        int searchIndex, neighborIndex, col, row, numberOfChanges=0;

       /* Do first row */
        row = minSearchRow - 1;
        if (row >= 0)
        {
           /* Do first col */
            col = minSearchCol-1;
            if (col >= 0)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col+1) + (row+1)*ncols;
                if ((sourceImage->get_data(col,row,0) == searchValue ) && 
                    (display_highlight[neighborIndex] ))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do last col */
            col = maxSearchCol+1;
            if (col < ncols)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col-1) + (row+1)*ncols;
                if ((sourceImage->get_data(col,row,0) == searchValue ) && 
                    (display_highlight[neighborIndex] ))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do remaining cols */
            for (col=minSearchCol; col <= maxSearchCol; col++)
            {
                searchIndex = col + row*ncols;
                if (sourceImage->get_data(col,row,0) == searchValue )
                {
                    neighborIndex = (col-1) + (row+1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                }
            }
        }

       /* Do last row */
        row = maxSearchRow + 1;
        if (row < nrows)
        {
           /* Do first col */
            col = minSearchCol-1;
            if (col >= 0)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col+1) + (row-1)*ncols;
                if ((sourceImage->get_data(col,row,0) == searchValue ) && 
                    (display_highlight[neighborIndex] ))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do last col */
            col = maxSearchCol+1;
            if (col < ncols)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col-1) + (row-1)*ncols;
                if ((sourceImage->get_data(col,row,0) == searchValue ) && 
                    (display_highlight[neighborIndex] ))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do remaining cols */
            for (col=minSearchCol; col <= maxSearchCol; col++)
            {
                searchIndex = col + row*ncols;
                if (sourceImage->get_data(col,row,0) == searchValue )
                {
                    neighborIndex = (col-1) + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                }
            }
        }

       /* Do remaining rows */
        for (row=minSearchRow; row <= maxSearchRow; row++)
        {
           /* Do first col */
            col = minSearchCol-1;
            if (col >= 0)
            {
                searchIndex = col + row*ncols;
                if (sourceImage->get_data(col,row,0) == searchValue )
                {
                    neighborIndex = col + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else
                    {
                        neighborIndex = col + row*ncols;
                        if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else
                        {
                            neighborIndex = col + (row+1)*ncols;
                            if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                        }
                    }
                }
            }

           /* Do last col */
            col = maxSearchCol+1;
            if (col < ncols)
            {
                searchIndex = col + row*ncols;
                if (sourceImage->get_data(col,row,0) == searchValue )
                {
                    neighborIndex = (col-1) + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else
                    {
                        neighborIndex = (col-1) + row*ncols;
                        if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else
                        {
                            neighborIndex = (col-1) + (row+1)*ncols;
                            if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                        }
                    }
                }
            }

           /* Do remaining cols */
            for (col=minSearchCol; col <= maxSearchCol; col++)
            {
                searchIndex = col + row*ncols;
                if ((sourceImage->get_data(col,row,0) == searchValue ) && 
                    (!display_highlight[searchIndex]))
                {
                    neighborIndex = (col-1) + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else
                    {
                        neighborIndex = (col-1) + row*ncols;
                        if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else
                        {
                            neighborIndex = (col-1) + (row+1)*ncols;
                            if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                        }
                    }
                }
            }
        }

        return numberOfChanges;
  }

  int DisplayImage::propagate(int minSearchCol, int maxSearchCol, int minSearchRow, int maxSearchRow)
  {
        int searchIndex, neighborIndex, col, row, numberOfChanges=0;

       /* Do first row */
        row = minSearchRow - 1;
        if (row >= 0)
        {
           /* Do first col */
            col = minSearchCol-1;
            if (col >= 0)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col+1) + (row+1)*ncols;
                if ((temp_highlight[searchIndex]) && (display_highlight[neighborIndex]))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do last col */
            col = maxSearchCol+1;
            if (col < ncols)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col-1) + (row+1)*ncols;
                if ((temp_highlight[searchIndex] ) && (display_highlight[neighborIndex] ))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do remaining cols */
            for (col=minSearchCol; col <= maxSearchCol; col++)
            {
                searchIndex = col + row*ncols;
                if (temp_highlight[searchIndex] )
                {
                    neighborIndex = (col-1) + (row+1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                }
            }
        }

       /* Do last row */
        row = maxSearchRow + 1;
        if (row < nrows)
        {
           /* Do first col */
            col = minSearchCol-1;
            if (col >= 0)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col+1) + (row-1)*ncols;
                if ((temp_highlight[searchIndex] ) && (display_highlight[neighborIndex] ))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do last col */
            col = maxSearchCol+1;
            if (col < ncols)
            {
                searchIndex = col + row*ncols;
                neighborIndex = (col-1) + (row-1)*ncols;
                if ((temp_highlight[searchIndex] ) && (display_highlight[neighborIndex] ))
                {
                    if (!display_highlight[searchIndex])
                    {
                        display_highlight[searchIndex] = true ;
                        numberOfChanges++;
                    }
                }
            }
           /* Do remaining cols */
            for (col=minSearchCol; col <= maxSearchCol; col++)
            {
                searchIndex = col + row*ncols;
                if (temp_highlight[searchIndex] )
                {
                    neighborIndex = (col-1) + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                }
            }
        }

       /* Do remaining rows */
        for (row=minSearchRow; row <= maxSearchRow; row++)
        {
           /* Do first col */
            col = minSearchCol-1;
            if (col >= 0)
            {
                searchIndex = col + row*ncols;
                if (temp_highlight[searchIndex] )
                {
                    neighborIndex = col + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else
                    {
                        neighborIndex = col + row*ncols;
                        if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else
                        {
                            neighborIndex = col + (row+1)*ncols;
                            if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                        }
                    }
                }
            }

           /* Do last col */
            col = maxSearchCol+1;
            if (col < ncols)
            {
                searchIndex = col + row*ncols;
                if (temp_highlight[searchIndex] )
                {
                    neighborIndex = (col-1) + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else
                    {
                        neighborIndex = (col-1) + row*ncols;
                        if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else
                        {
                            neighborIndex = (col-1) + (row+1)*ncols;
                            if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                        }
                    }
                }
            }

           /* Do remaining cols */
            for (col=minSearchCol; col <= maxSearchCol; col++)
            {
                searchIndex = col + row*ncols;
                if ((temp_highlight[searchIndex] ) && (!display_highlight[searchIndex]))
                {
                    neighborIndex = (col-1) + (row-1)*ncols;
                    if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else if (display_highlight[neighborIndex++])
                    {
                        if (!display_highlight[searchIndex])
                        {
                            display_highlight[searchIndex] = true ;
                            numberOfChanges++;
                        }
                    }
                    else
                    {
                        neighborIndex = (col-1) + row*ncols;
                        if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else if (display_highlight[neighborIndex++])
                        {
                            if (!display_highlight[searchIndex])
                            {
                                display_highlight[searchIndex] = true ;
                                numberOfChanges++;
                            }
                        }
                        else
                        {
                            neighborIndex = (col-1) + (row+1)*ncols;
                            if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                            else if (display_highlight[neighborIndex++])
                            {
                                if (!display_highlight[searchIndex])
                                {
                                    display_highlight[searchIndex] = true ;
                                    numberOfChanges++;
                                }
                            }
                        }
                    }
                }
            }
        }

        return numberOfChanges;
  }

} // namespace CommonTilton
