#include "utils.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QGuiApplication>
#include <QClipboard>
#include <QSvgRenderer>
#include <QPainter>
#include <QScreen>
// #include <QDebug>
#include "windows.h"

void savePixmap(const QPixmap& pixmap, QWidget* parent)
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString now = QDateTime::currentDateTime().toString("MM-dd-yyyy HHmmss");
    QString defaultPath = dir + "/screenshot " + now + ".png";

    QString path = QFileDialog::getSaveFileName(parent, "Save Screenshot", defaultPath, "Images (*.png)");
    if (path.isEmpty()) return;

    pixmap.save(path);
}

void copyPixmap(const QPixmap& pixmap) { QGuiApplication::clipboard()->setPixmap(pixmap); }

QIcon tintedIcon(const QString& path, const QColor& color, int size)
{
    QSvgRenderer renderer(path);
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pixmap(size * dpr, size * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    return QIcon(pixmap);
}

QIcon tintedIcon(const QString& path, int size) { return tintedIcon(path, Qt::white, size); }

QIcon colorSwatchIcon(const QColor& color, int size)
{
    QPixmap px(size, size);
    px.fill(color);
    return QIcon(px);
}

QString vkToDisplayString(unsigned int vk)
{
    // qDebug() << vk << " " << VK_SNAPSHOT;

    // GetKeyNameText returns "Sys Req" for VK_SNAPSHOT, which is wrong
    if (vk == VK_SNAPSHOT) return "Print Screen";

    // UINT is an unsigned int, this should keep utils.h qt only... I think...
    // not really sure what the convention is here
    UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    switch (vk)
    {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_END:
        case VK_HOME:
        case VK_INSERT:
        case VK_DELETE:
        case VK_DIVIDE:
        case VK_NUMLOCK: scanCode |= 0x100; break;
    }
    wchar_t buf[64] = {};
    GetKeyNameTextW(static_cast<LONG>(scanCode << 16), buf, 64);
    return QString::fromWCharArray(buf);
}

QString hotkeyToDisplayString(unsigned int vk, unsigned int modifiers)
{
    QString result;
    if (modifiers & MOD_CONTROL) result += "Ctrl+";
    if (modifiers & MOD_ALT) result += "Alt+";
    if (modifiers & MOD_SHIFT) result += "Shift+";
    if (modifiers & MOD_WIN) result += "Win+";
    result += vkToDisplayString(vk);
    return result;
}

QPixmap grabVirtualDesktop()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect geo = screen->virtualGeometry();
    return screen->grabWindow(0, geo.x(), geo.y(), geo.width(), geo.height());
}

QRect physicalCrop(const QRect& logicalRect, qreal dpr)
{
    return QRect(
        int(logicalRect.x() * dpr), int(logicalRect.y() * dpr), int(logicalRect.width() * dpr),
        int(logicalRect.height() * dpr)
    );
}

QPixmap grabFullscreenAtCursor()
{
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) screen = QGuiApplication::primaryScreen();
    return grabVirtualDesktop().copy(physicalCrop(screen->geometry(), screen->devicePixelRatio()));
}