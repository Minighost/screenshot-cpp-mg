#pragma once
#include <QWidget>
#include <QPixmap>
#include <QRect>
#include <windows.h>

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
    QPixmap _screenshot;
    QPixmap _screenshotLogical;
    QRect _highlightRect;
    qreal _dpr;

    void _capture();
    void _cleanClose();
};