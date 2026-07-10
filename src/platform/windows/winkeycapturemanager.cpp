#include "winkeycapturemanager.h"
#include <windows.h>

void WinKeyCaptureManager::startCapture(WId windowId)
{
    _capturing = true;
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = (HWND)windowId;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

void WinKeyCaptureManager::stopCapture()
{
    _capturing = false;
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_REMOVE;
    rid.hwndTarget = NULL;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

bool WinKeyCaptureManager::handleNativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    if (!_capturing) return false;

    MSG* msg = static_cast<MSG*>(message);
    if (msg->message != WM_INPUT) return false;

    UINT size = 0;
    GetRawInputData((HRAWINPUT)msg->lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
    QByteArray buffer(size, 0);
    GetRawInputData((HRAWINPUT)msg->lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));

    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
    if (raw->header.dwType != RIM_TYPEKEYBOARD) return false;

    const RAWKEYBOARD& kb = raw->data.keyboard;
    if (!(kb.Flags & RI_KEY_BREAK)) return false;

    switch (kb.VKey)
    {
        case VK_CONTROL:
        case VK_SHIFT:
        case VK_MENU:
        case VK_LWIN:
        case VK_RWIN: return false;
        default: break;
    }

    quint32 modifiers = MOD_NOREPEAT;
    if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= MOD_CONTROL;
    if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= MOD_SHIFT;
    if (GetKeyState(VK_MENU) & 0x8000) modifiers |= MOD_ALT;
    if (GetKeyState(VK_LWIN) & 0x8000) modifiers |= MOD_WIN;
    if (GetKeyState(VK_RWIN) & 0x8000) modifiers |= MOD_WIN;

    emit keyCaptured(kb.VKey, modifiers);
    return true;
}