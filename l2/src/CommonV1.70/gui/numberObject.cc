// numberObject.cc
#include "numberObject.h"

#include <iostream>
using namespace std;

namespace CommonTilton
{
  // Constructors
  NumberObject::NumberObject(const Glib::ustring& label)
  {
    initialize(label,true);
  }

  NumberObject::NumberObject(const Glib::ustring& label, const bool requiredFlag)
  {
    initialize(label,requiredFlag);
  }

  // Destructor...
  NumberObject::~NumberObject() 
  {
  }

  void NumberObject::initialize(const Glib::ustring& label, const bool requiredFlag)
  {
    activeFlag = requiredFlag;

    add(hBox);
    
    if (requiredFlag)
    {
      numberLabel.set_label(label);
      numberLabel.set_alignment(0.0,0.5);
      hBox.pack_start(numberLabel);
    }
    else
    {
      activeButton.set_label(label);
      activeButton.set_active(activeFlag);
      activeButton.set_alignment(0.0,0.5);
      activeButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &NumberObject::on_activeButton_checked) );
      hBox.pack_start(activeButton);
    }
    numberEntry.set_alignment(1); // Horizontally align to right.
    numberEntry.signal_activate().connect( sigc::mem_fun(*this,
                                      &NumberObject::on_numberEntry_activate) );
    hBox.pack_start(numberEntry);
  }

  NumberObject::type_signal_activate NumberObject::signal_activate()
  {
    return m_signal_activate;
  }

  NumberObject::type_signal_activated NumberObject::signal_activated()
  {
    return m_signal_activated;
  }

  void NumberObject::on_numberEntry_activate()
  {

    m_signal_activate.emit();

    return;
  }

  void NumberObject::on_activeButton_checked()
  {

    activeFlag = activeButton.get_active();
    if (activeFlag)
    {
      numberEntry.show();
      m_signal_activated.emit();
    }
    else
      numberEntry.hide();
  }

  void NumberObject::set_activeFlag(const bool value)
  {
    activeFlag = value;
    activeButton.set_active(activeFlag);
    if (activeFlag)
      numberEntry.show();
    else
      numberEntry.hide();
  }

  bool NumberObject::value_valid()
  {
    return (numberEntry.get_text() != "");
  }

  int NumberObject::get_value()
  {
    return atoi(numberEntry.get_text().c_str());
  }

  float NumberObject::get_fvalue()
  {
    return (float) atof(numberEntry.get_text().c_str());
  }

  void NumberObject::set_value(const int value)
  {
    char ctext[12];
    sprintf(ctext,"%d",value);
    numberEntry.set_text(ctext);
  }

  void NumberObject::set_fvalue(const float value)
  {
    char ctext[12];
    sprintf(ctext,"%f",value);
    numberEntry.set_text(ctext);
  }

  void NumberObject::clear()
  {
    numberEntry.set_text("");
  }

} // namespace CommonTilton
