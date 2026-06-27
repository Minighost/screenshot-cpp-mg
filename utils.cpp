#include "utils.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QGuiApplication>
#include <QClipboard>

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