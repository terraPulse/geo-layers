#ifndef FILEOBJECT_H
#define FILEOBJECT_H

#include <gtkmm.h>

namespace CommonTilton
{
  class FileObject : public Gtk::Frame
  {
    public:
        FileObject(const Glib::ustring& label, const Glib::ustring& title, 
                   const Gtk::FileChooserAction action);
        FileObject(const Glib::ustring& label, const Glib::ustring& title, 
                   const Gtk::FileChooserAction action, const bool requiredFlag);
        FileObject(const Glib::ustring& label, const Glib::ustring& title, 
                   const Gtk::FileChooserAction action, const bool requiredFlag,
                   const Glib::ustring& fileName);
        virtual ~FileObject();

        bool get_activeFlag() const { return activeFlag; }
        void set_activeFlag(const bool value);
        bool get_validFlag() const { return validFlag; }
        Glib::ustring get_filename();
        void set_filename(const Glib::ustring& fileName);
        void set_current_folder(const Glib::ustring& folder);
        void add_shortcut_folder(const Glib::ustring& folder);

      //Signal accessors:
        typedef sigc::signal<void> type_signal_selection_changed;
        type_signal_selection_changed signal_selection_changed();
        typedef sigc::signal<void> type_signal_activated;
        type_signal_activated signal_activated();

    protected:
      //Signal handlers:
        virtual void on_fileDialogButton_clicked();
        virtual void on_activeButton_checked();

      //Signal accessors:
        type_signal_selection_changed m_signal_selection_changed;
        type_signal_activated m_signal_activated;

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::Label fileLabel;
        Gtk::Button fileDialogButton;
        Gtk::CheckButton activeButton;

    private:
        bool activeFlag, validFlag;
        Gtk::FileChooserAction dialogAction;
        Glib::ustring dialogTitle, current_folder, shortcut_folder;
        void initialize(const Glib::ustring& label, const Glib::ustring& title, 
                        const Gtk::FileChooserAction action, const bool requiredFlag,
                        const Glib::ustring& fileName);
  };

} // namespace CommonTilton
#endif /* FILEOBJECT_H */

