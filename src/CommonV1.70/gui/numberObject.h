#ifndef NUMBEROBJECT_H
#define NUMBEROBJECT_H

#include <gtkmm.h>

namespace CommonTilton
{
  class NumberObject : public Gtk::Frame
  {
    public:
        NumberObject(const Glib::ustring& label);
        NumberObject(const Glib::ustring& label, const bool requiredFlag);
        virtual ~NumberObject();
    
        bool get_activeFlag() const { return activeFlag; }
        void set_activeFlag(const bool value);
        bool value_valid();
        int get_value();
        float get_fvalue();
        void set_value(const int value);
        void set_fvalue(const float fvalue);
        void clear();

      //Signal accessors:
        typedef sigc::signal<void> type_signal_activate;
        type_signal_activate signal_activate();
        typedef sigc::signal<void> type_signal_activated;
        type_signal_activated signal_activated();

    protected:
      //Signal handlers:
        virtual void on_numberEntry_activate();
        virtual void on_activeButton_checked();

      //Signal accessors:
        type_signal_activate m_signal_activate;
        type_signal_activated m_signal_activated;

      //Member widgets:
        Gtk::HBox  hBox;
        Gtk::Label numberLabel;
        Gtk::Entry numberEntry;
        Gtk::CheckButton activeButton;

    private:
        bool activeFlag;
        void initialize(const Glib::ustring& label, const bool requiredFlag);
  };

} // namespace CommonTilton
#endif /* NUMBEROBJECT_H */

