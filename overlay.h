#pragma once
#include <QWidget>

class CaptureOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureOverlay(QWidget *parent = nullptr);
    void show();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};