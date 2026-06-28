#pragma once
#include <functional>
#include <QWidget>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QToolBar>
#include <QLabel>
#include <QUndoStack>

enum class PreviewTool
{
    Pan,
    FreeDraw,
};

class PreviewView : public QGraphicsView
{
    Q_OBJECT

   public:
    explicit PreviewView(
        QGraphicsScene* scene, QWidget* parent = nullptr, std::function<void(qreal)> onZoom = nullptr,
        QUndoStack* undoStack = nullptr
    );
    void setTool(PreviewTool tool);

   protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

   private:
    std::function<void(qreal)> _onZoom;
    QUndoStack* _undoStack;
    PreviewTool _tool;
    QGraphicsPathItem* _currentStroke;
    QPainterPath _currentPath;
};

class PreviewWindow : public QWidget
{
    Q_OBJECT

   public:
    explicit PreviewWindow();
    void setPixmap(const QPixmap& pixmap);

   protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

   private:
    QGraphicsScene* _scene;
    QGraphicsPixmapItem* _pixmapItem;
    QUndoStack* _undoStack;
    PreviewView* _view;
    QToolBar* _toolbar;
    QLabel* _zoomLabel;
    QAction* _drawAction;

    void _save();
    void _copy();
    void _resetView();
    void _resetSize();
    void _updateZoomLabel(qreal scale);
    QPixmap _renderScene();
};