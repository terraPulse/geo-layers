#ifndef LABELREGION_H
#define LABELREGION_H

#include <defines.h>
#include <image/image.h>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{

  class LabelRegion : public Gtk::Window
  {
    public:
      LabelRegion();
      virtual ~LabelRegion();
      void set_text_and_colormap();
      unsigned int get_red_color(const int& index); 
      unsigned int get_green_color(const int& index); 
      unsigned int get_blue_color(const int& index); 
      void write_ascii_out();
      unsigned char get_highlight_label() const { return highlight_label; }

      //Signal accessors:
      typedef sigc::signal<void> type_signal_activated;
      type_signal_activated signal_activated();
      typedef sigc::signal<void> type_signal_color_set;
      type_signal_color_set signal_color_set();
      typedef sigc::signal<void> type_signal_undo;
      type_signal_undo signal_undo();

    protected:
      //Signal handlers:
      virtual void on_menu_undo();
      virtual void on_menu_close();
      virtual void on_menu_others();
      void on_entry_activate(unsigned char highlight_label);
      void on_color_set();

      //Signal accessors:
      type_signal_activated m_signal_activated;
      type_signal_color_set m_signal_color_set;
      type_signal_undo m_signal_undo;

      //Child widgets:
      Gtk::VBox vBox;

      Gtk::Table colorTable;
      Gtk::Label colorsLabel[3], namesLabel[3];
      Gdk::Color color[NUMBER_OF_LABELS];
      Gtk::ColorButton colorButton[NUMBER_OF_LABELS];
      Gtk::Entry entry[NUMBER_OF_LABELS];

      Glib::RefPtr<Gtk::UIManager> m_refUIManager;
      Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
      unsigned char highlight_label;
  };

} // HSEGTilton

#endif /* LABELREGION_H */

