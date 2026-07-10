#pragma once
#include <QWidget>
#include <QPixmap>
#include <QRect>
#include <windows.h>
#include "platform/platform.h"

class WindowOverlay : public QWidget
{
    Q_OBJECT
   public:
    explicit WindowOverlay(QWidget* parent = nullptr);
    void show();

   protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    qreal _dpr;
    WindowPicker* _windowPicker;
    QPixmap _screenshot;
    QPixmap _screenshotLogical;
    QRect _highlightRect;

    void _capture();
    void _cleanClose();
};