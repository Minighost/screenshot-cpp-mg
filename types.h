#pragma once
#include <QtTypes>
#include <windows.h>

struct HotkeyData
{
    UINT vk = 0;
    UINT modifiers = MOD_NOREPEAT;
    bool isEmpty() const { return vk == 0; }
    bool operator==(const HotkeyData& o) const { return vk == o.vk && modifiers == o.modifiers; }
    bool operator!=(const HotkeyData& o) const { return !(*this == o); }
};

enum HotkeyId : quint32
{
    Overlay = 1,
    Fullscreen = 2,
    WindowCapture = 3
};

enum class CaptureAction
{
    Copy,
    Save,
    Preview
};