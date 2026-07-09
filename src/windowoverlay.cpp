#include "windowoverlay.h"
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QCursor>
#include <QSettings>
// #include <QDebug>
#include <dwmapi.h>
#include "utils.h"
#include "preview.h"

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
    if (FAILED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &wr, sizeof(wr)))) { GetWindowRect(hwnd, &wr); }

    if (PtInRect(&wr, data->pt))
    {
        data->result = hwnd;
        return FALSE;
    }
    return TRUE;
}

WindowOverlay::WindowOverlay(QWidget* parent)
    : QWidget(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool), _dpr(1.0)
{
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void WindowOverlay::show()
{
    _highlightRect = QRect();
    _screenshot = grabVirtualDesktop();

    QScreen* screen = QGuiApplication::primaryScreen();
    _dpr = screen->devicePixelRatio();
    QRect virtualGeo = screen->virtualGeometry();

    _screenshotLogical =
        _screenshot.scaled(virtualGeo.width(), virtualGeo.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    setGeometry(virtualGeo);
    QWidget::show();
    activateWindow();
    setFocus();
}

void WindowOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (!_screenshotLogical.isNull()) painter.drawPixmap(rect(), _screenshotLogical);

    painter.fillRect(rect(), QColor(0, 0, 0, 100));

    if (_highlightRect.isNull() || !_highlightRect.isValid()) return;

    // draw the highlighted window region undarkenened
    painter.drawPixmap(_highlightRect, _screenshot, physicalCrop(_highlightRect, _dpr));

    // draw highlight border
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(0, 120, 215, 230), 2));
    painter.drawRect(_highlightRect);
}

void WindowOverlay::mouseMoveEvent(QMouseEvent* event)
{
    QPoint screenPos = mapToGlobal(event->pos());

    HitTestData data;
    data.pt = {screenPos.x(), screenPos.y()};
    data.ownHwnd = (HWND)winId();
    data.result = nullptr;

    EnumWindows(findWindowAtPoint, reinterpret_cast<LPARAM>(&data));

    if (data.result)
    {
        RECT wr;
        if (FAILED(DwmGetWindowAttribute(data.result, DWMWA_EXTENDED_FRAME_BOUNDS, &wr, sizeof(wr))))
            GetWindowRect(data.result, &wr);
        QRect newRect = QRect(QPoint(wr.left, wr.top), QPoint(wr.right, wr.bottom)).normalized();
        if (newRect != _highlightRect)
        {
            _highlightRect = newRect;
            update();
        }
    }
    else if (!_highlightRect.isNull())
    {
        _highlightRect = QRect();
        update();
    }
}

void WindowOverlay::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        _cleanClose();
        return;
    }
    if (event->button() != Qt::LeftButton) return;
    _capture();
}

void WindowOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) _cleanClose();
}

void WindowOverlay::_capture()
{
    if (_highlightRect.isNull() || _screenshot.isNull()) return;
    QPixmap crop = _screenshot.copy(physicalCrop(_highlightRect, _dpr));
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    CaptureAction action = static_cast<CaptureAction>(settings.value("action_window", 0).toInt());
    performCaptureAction(crop, action);
    _cleanClose();
}

void WindowOverlay::_cleanClose() { close(); }