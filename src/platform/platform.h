#pragma once
#include "windowpicker.h"
#include "keycapturemanager.h"
#include "hotkeymanager.h"

WindowPicker* createWindowPicker(QObject* parent = nullptr);
KeyCaptureManager* createKeyCaptureManager(QObject* parent = nullptr);
HotkeyManager* createHotkeyManager(QObject* parent = nullptr);