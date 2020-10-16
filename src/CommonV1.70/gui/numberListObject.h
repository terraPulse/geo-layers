#ifndef NUMBERLISTOBJECT_H
#define NUMBERLISTOBJECT_H

#include <gtkmm.h>

namespace CommonTilton
{
  class NumberListObject : public Gtk::Frame
  {
    public:
        NumberListObject(const Glib::ustring& label);
        NumberListObject(const Glib::ustring& label, const bool requiredFlag);
        virtual ~NumberListObject();
    
        bool get_activeFlag() const { return activeFlag; }
        void set_activeFlag(const bool value);
        unsigned short int get_size();
        int get_entry(const int entry_index);
        float get_fentry(const int entry_index);
        void clear();

      //Signal accessors:
        typedef sigc::signal<void> type_signal_activated;
        type_signal_activated signal_activated();

    protected:
      //Signal handlers:
        virtual void on_activeButton_checked();

      //Signal accessors:
        type_signal_activated m_signal_activated;

      //Member widgets:
        Gtk::VBox  vBox;
        Gtk::Label numberListLabel;
        Gtk::Entry numberListEntry;
        Gtk::CheckButton activeButton;

    private:
        bool activeFlag;
        void initialize(const Glib::ustring& label, const bool requiredFlag);
  };

} // namespace CommonTilton
#endif /* NUMBERLISTOBJECT_H */
