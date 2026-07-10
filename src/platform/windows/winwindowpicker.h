#pragma once
#include "../windowpicker.h"
#include <windows.h>

class WinWindowPicker : public WindowPicker
{
    Q_OBJECT
   public:
    explicit WinWindowPicker(QObject* parent = nullptr) : WindowPicker(parent) {}

    QRect windowAtPoint(QPoint screenPos, WId excludeWindow) override;
};