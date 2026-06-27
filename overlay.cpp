#include "overlay.h"

CaptureOverlay::CaptureOverlay(QWidget *parent)
    : QWidget(parent)
{
}

void CaptureOverlay::show()
{
    QWidget::show();
}

void CaptureOverlay::paintEvent(QPaintEvent *event) {}
void CaptureOverlay::mousePressEvent(QMouseEvent *event) {}
void CaptureOverlay::mouseMoveEvent(QMouseEvent *event) {}
void CaptureOverlay::mouseReleaseEvent(QMouseEvent *event) {}
void CaptureOverlay::keyPressEvent(QKeyEvent *event) {}