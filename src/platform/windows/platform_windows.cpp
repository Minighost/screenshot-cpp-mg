#include "../platform.h"
#include "winwindowpicker.h"

WindowPicker* createWindowPicker(QObject* parent) { return new WinWindowPicker(parent); }