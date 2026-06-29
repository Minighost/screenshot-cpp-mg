#pragma once
#include <QPixmap>
#include <QWidget>
#include <QIcon>
#include <QColor>

void savePixmap(const QPixmap& pixmap, QWidget* parent = nullptr);
void copyPixmap(const QPixmap& pixmap);
QIcon tintedIcon(const QString& path, const QColor& color = Qt::white);