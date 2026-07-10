#pragma once
#include "windowpicker.h"
#include "keycapturemanager.h"

WindowPicker* createWindowPicker(QObject* parent = nullptr);
KeyCaptureManager* createKeyCaptureManager(QObject* parent = nullptr);