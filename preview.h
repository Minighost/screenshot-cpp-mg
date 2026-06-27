#pragma once
#include <functional>
#include <QWidget>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QToolBar>
#include <QLabel>

class PreviewView : public QGraphicsView
{
    Q_OBJECT

   public:
    explicit PreviewView(QGraphicsScene* scene, QWidget* parent = nullptr, std::function<void(qreal)> onZoom = nullptr);

   protected:
    void wheelEvent(QWheelEvent* event) override;

   private:
    std::function<void(qreal)> _onZoom;
};

class PreviewWindow : public QWidget
{
    Q_OBJECT

   public:
    explicit PreviewWindow();
    void setPixmap(const QPixmap& pixmap);

   protected:
    void keyPressEvent(QKeyEvent* event);

   private:
    QGraphicsScene* _scene;
    QGraphicsPixmapItem* _pixmap_item;
    PreviewView* _view;
    QToolBar* _toolbar;
    QLabel* _zoomLabel;

    void _save();
    void _copy();
    void _resetView();
    void _updateZoomLabel(qreal scale);
};