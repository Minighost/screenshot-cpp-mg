#pragma once
#include <QPixmap>
#include <QWidget>
#include <QIcon>
#include <QColor>

void savePixmap(const QPixmap& pixmap, QWidget* parent = nullptr);
void copyPixmap(const QPixmap& pixmap);
QIcon tintedIcon(const QString& path, const QColor& color = Qt::white, int size = 24);
QIcon tintedIcon(const QString& path, int size);
QIcon colorSwatchIcon(const QColor& color, int size = 24);
QString vkToDisplayString(unsigned int vk);
QString hotkeyToDisplayString(unsigned int vk, unsigned int modifiers);
QRect physicalCrop(const QRect& logicalRect, qreal dpr);
QPixmap grabFullscreenAtCursor();