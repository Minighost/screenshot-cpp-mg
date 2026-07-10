#include "winwindowpicker.h"
#include <dwmapi.h>

struct HitTestData
{
    POINT pt;
    HWND ownHwnd;
    HWND result;
};

static BOOL CALLBACK findWindowAtPoint(HWND hwnd, LPARAM lParam)
{
    HitTestData* data = reinterpret_cast<HitTestData*>(lParam);
    if (hwnd == data->ownHwnd) return TRUE;
    if (!IsWindowVisible(hwnd)) return TRUE;

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if ((exStyle & WS_EX_TOOLWINDOW) && (exStyle & WS_EX_TOPMOST)) return TRUE;

    RECT wr;
    if (FAILED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &wr, sizeof(wr)))) GetWindowRect(hwnd, &wr);

    if (PtInRect(&wr, data->pt))
    {
        data->result = hwnd;
        return FALSE;
    }
    return TRUE;
}

QRect WinWindowPicker::windowAtPoint(QPoint screenPos, WId excludeWindow)
{
    HitTestData data;
    data.pt = {screenPos.x(), screenPos.y()};
    data.ownHwnd = (HWND)excludeWindow;
    data.result = nullptr;

    EnumWindows(findWindowAtPoint, reinterpret_cast<LPARAM>(&data));

    if (!data.result) return QRect();

    RECT wr;
    if (FAILED(DwmGetWindowAttribute(data.result, DWMWA_EXTENDED_FRAME_BOUNDS, &wr, sizeof(wr))))
        GetWindowRect(data.result, &wr);

    return QRect(QPoint(wr.left, wr.top), QPoint(wr.right, wr.bottom)).normalized();
}