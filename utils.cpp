#include "utils.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QGuiApplication>
#include <QClipboard>
#include <QSvgRenderer>
#include <QPainter>

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

QIcon tintedIcon(const QString& path, const QColor& color)
{
    QSvgRenderer renderer(path);
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    return QIcon(pixmap);
}