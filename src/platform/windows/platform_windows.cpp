#include "../platform.h"
#include "winwindowpicker.h"
#include "winkeycapturemanager.h"
#include "winhotkeymanager.h"

WindowPicker* createWindowPicker(QObject* parent) { return new WinWindowPicker(parent); }
KeyCaptureManager* createKeyCaptureManager(QObject* parent) { return new WinKeyCaptureManager(parent); }
HotkeyManager* createHotkeyManager(QObject* parent) { return new WinHotkeyManager(parent); }