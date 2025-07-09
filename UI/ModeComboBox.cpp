#include "ModeComboBox.h"

ModeComboBox::ModeComboBox(QWidget* parent)
    : QComboBox(parent)
{
    addItem("cura");
    addItem("bambu");
} 