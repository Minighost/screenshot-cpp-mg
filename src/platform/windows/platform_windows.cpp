#include "../platform.h"
#include "winwindowpicker.h"
#include "winkeycapturemanager.h"

WindowPicker* createWindowPicker(QObject* parent) { return new WinWindowPicker(parent); }
KeyCaptureManager* createKeyCaptureManager(QObject* parent) { return new WinKeyCaptureManager(parent); }