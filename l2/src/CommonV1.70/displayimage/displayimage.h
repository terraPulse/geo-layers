#ifndef DISPLAYIMAGE_H
#define DISPLAYIMAGE_H

#define MAX_LINKED_IMAGES       12
#define MAX_VISIBLE_SIZE       512
#define MAX_THUMB_SIZE         128
#define TIME_SIZE               26 // Dimension size for vector used in timing buffer string

#include "../image/image.h"
#include "../point/point.h"
#include <gtkmm.h>

using namespace std;

namespace CommonTilton
{

  class DisplayImage : public Gtk::Window
  {
   public:
      DisplayImage();
      virtual ~DisplayImage();

      static void set_static_values(const Image& input_image, const string& folder);
      static void set_static_values(const Image& input_image, const string& folder, const bool& highlight_all_flag);
      static void set_static_values(const Image& input_image, const string& folder, const bool& highlight_all_flag, const bool& hlearn_flag);

    // For RGB Image and Region Mean Image (type 1)
      void init_rgb(Image& input_image, const Glib::ustring& title);
      void init_rgb(Image& input_image, Image& mask_image, const Glib::ustring& title);
      void reinit_rgb();
    // For Segmentation Classes Slice and Segmentation Objects Slice (type 2)
      void init_seg(Image& input_image, const Glib::ustring& title);
      void reinit_seg();
    // For Current Class Labels (type 3)
      void init_region_label(Image& input_image, const Glib::ustring& title);
      void init_region_label(Image& input_image, Image& mask_image, const Glib::ustring& title);
      void reinit_region_label();
    // For Hierarchical Boundary Map (type 4)
      void init_boundary(Image& input_image, const Glib::ustring& title);
      void reinit_boundary();
    // For Reference Images - (type 5) (Like type 6, but no scaling or masking)
      void init_reference(Image& input_image, const Glib::ustring& title);
    // For General Float or Double (single band) Images (e.g. Std. Dev. Maps, Boundary Pixel Ratio (BPRatio) Maps,
    // NDVI Images, NDSI Images, etc. - type 6) - Also can be used for short int or int (single band) images!
      void init_float(Image& input_image, const Glib::ustring& title);
      void init_float(Image& input_image, Image& mask_image, const Glib::ustring& title);
      void reinit_float();
    // For MODIS Snow Maps (type 7)
      void init_snow(Image& input_image, const Glib::ustring& title);
    // For MODIS Thin Cloud Mask (type 8)
      void init_thin_cloud(Image& input_image, const Glib::ustring& title);
    // For Revised Snow Map (type 9)
      void init_revised_snow(Image& input_image, const Glib::ustring& title);
      void reinit_revised_snow();
    // For Drawing a Path (type 10)
      void init_path(Image& input_image, vector<Point>& input_path, const Glib::ustring& title);
      void reinit_path(vector<Point>& input_path);
    // For Archaeological Site Detection Image (type 11)
      void init_detect(Image& detect_image, Image& site_location_image, const Glib::ustring& title);
      void reinit_detect();

    // Other public functions
      void copy_display(Image& copy_image);
      void revise_display(Image& revise_image);
      void revise_display(int& label);
      void link_image(DisplayImage *image);
      void set_click_location(int& col, int& row);
      bool get_click_location(int& col, int& row);
      void set_label_mask(const unsigned int& selLabel);
      bool set_display_highlight(const unsigned int& selLabel);
      bool set_display_highlight(const unsigned int& selLabel, Image& labelImage);
      bool set_display_highlight(const unsigned int& selLabel, Image& labelImage, const bool& clearFlag);
      bool set_display_highlight(const unsigned int& selLabel, Image* labelImage);
      bool set_display_highlight(const float& threshold, float* threshold_array);
      bool set_display_highlight(const float& threshold, float* threshold_array, const bool& thresholdOverUnderFlag);
      bool get_display_highlight(const int& col, const int& row);
      void reset_display_highlight();
      void highlight_region(int& colSelect, int& rowSelect);
      void highlight_region(Image* sourceImage, int& colSelect, int& rowSelect);
      void highlight_select_region(int& colSelect, int& rowSelect);
      void highlight_circle_roi(Cairo::RefPtr<Cairo::Context>& display_CairoContext);
      void clear_path();
      bool set_focus(const bool& focusAll);
      bool set_linked_focus(const bool& focusAll);
      void display_resize(const int& width, const int& height);
      double get_X_zoom_factor() const { return X_zoom_factor; }
      double get_Y_zoom_factor() const { return Y_zoom_factor; }
      void registered_data_copy(Image& destImage, const int& band)
           { destImage.registered_data_copy(band,displayImage,band); displayImage.close(); return; }
      void remove_display_file();
      void set_button_press_monitor_flag(const bool& value);

      //Signal accessors:
      typedef sigc::signal<void> type_signal_button_press_event;
      type_signal_button_press_event signal_button_press_event();
      typedef sigc::signal<void> type_signal_circle_roi_event;
      type_signal_circle_roi_event signal_circle_roi_event();

    protected:
    //Signal handlers:
      virtual void on_menu_save_png();
      virtual void on_menu_save_data();
      virtual void on_menu_zoom_in();
      virtual void on_menu_zoom_out();
      virtual void on_menu_circle_roi();
      virtual void on_menu_select_region();
      virtual void on_menu_select_off();
      virtual void on_menu_help();
      virtual void on_menu_file_close();
      bool on_thumb_button_press_event(GdkEventButton* event);
      bool on_thumb_motion_notify_event(GdkEventMotion* event);
      bool on_thumb_button_release_event(GdkEventButton* event);
#ifdef GTKMM3
      bool on_thumb_draw(const Cairo::RefPtr<Cairo::Context>& thumb_CairoContext);
#else
      bool on_thumb_expose_event(GdkEventExpose* event);
#endif
      bool on_display_button_press_event(GdkEventButton* event);
      bool on_display_motion_notify_event(GdkEventMotion* event);
      bool on_display_button_release_event(GdkEventButton* event);
#ifdef GTKMM3
      bool on_display_draw(const Cairo::RefPtr<Cairo::Context>& display_CairoContext);
#else
      bool on_display_expose_event(GdkEventExpose* event);
#endif
      void on_hadjustment_value_changed();
      void on_vadjustment_value_changed();
      void on_window_check_resize();
      bool on_window_enter_notify_event(GdkEventCrossing* event);
      bool on_window_leave_notify_event(GdkEventCrossing* event);

    //Signal accessors:
      type_signal_button_press_event m_signal_button_press_event;
      type_signal_circle_roi_event m_signal_circle_roi_event;

    //Child widgets:
      Gtk::VBox m_Box;
      Gtk::DrawingArea thumb_DrawingArea;
      Gtk::DrawingArea display_DrawingArea;
      Gtk::ScrolledWindow display_ScrolledWindow;
#ifdef GTKMM3
      Glib::RefPtr<Gtk::Adjustment> hadjustment, vadjustment;
#else
      Gtk::Adjustment *hadjustment, *vadjustment;
#endif
      Gtk::Label pixel_Label, pixel_Label2;

      Glib::RefPtr<Gtk::UIManager> m_refUIManager;
      Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

      Glib::RefPtr<Gdk::Pixbuf> full_Pixbuf, display_Pixbuf, thumb_Pixbuf;

    private:
      bool set_display_file();
      void calc_defaults();
      double offset_for_rounding(const double& value);
      void init();
      void set_display_thumb_pixbuf();
      void reset_display_thumb_pixbuf();
      void display_zoom_factor_reset(const double& factor);
      void set_pixel_label();
      void set_pixel_label(const int& colPos, const int& rowPos);
      void clone_cursor(gdouble& x, gdouble& y);
      void clear_cursor();
      void thumb_refresh();
      void display_refresh(vector<Point>& input_path);
      void display_refresh();
      void draw_path(const Cairo::RefPtr<Cairo::Context>& display_CairoContext);
      int propagate(Image* sourceImage, int searchValue, 
                    int minSearchCol, int maxSearchCol, int minSearchRow, int maxSearchRow);
      int propagate(int minSearchCol, int maxSearchCol, int minSearchRow, int maxSearchRow);
      double hadjustment_value, vadjustment_value;
      string display_file;
      Image displayImage, *inputImage, *maskImage;
      RHSEGDType dtype;
      bool *display_highlight, *temp_highlight;
      unsigned char display_type, *display_data;
      unsigned int  numberLinked;
      double zoom_factor, relative_X_zoom_factor, relative_Y_zoom_factor, thumb_factor;
      int colPosition, rowPosition, colClick, rowClick;
      bool clickFlag, resizeFlag, cross_hair_flag;
      bool mask_valid, circle_roi_flag, select_region_flag, button_pressed_flag, button_press_monitor_flag;
      int ncols, nrows, display_ncols, display_nrows, thumb_ncols, thumb_nrows;
      int window_ncols, window_nrows, visible_ncols, visible_nrows;
      DisplayImage *linkedImages[MAX_LINKED_IMAGES];
      int    rgb_image_stretch;
      float  range[2];
      double scale, offset;
      double *band_min, *band_max;
      vector<Point> path;
      static bool circle_roi_highlight_all_flag, geotransform_flag, hseglearn_flag;
      static int input_ncols, input_nrows, base_ncols, base_nrows;
      static double base_X_gsd, base_Y_gsd, X_offset, Y_offset;
      static double X_zoom_factor, Y_zoom_factor;
      static string current_folder, shortcut_folder;
  };

} // CommonTilton

#endif /* DISPLAYIMAGE_H */
