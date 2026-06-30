#pragma once
#include <functional>
#include <QWidget>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QToolBar>
#include <QLabel>
#include <QUndoStack>
#include <QColor>

enum class PreviewTool
{
    Pan,
    FreeDraw,
    Rectangle,
    FilledRectangle,
    Ellipse,
    FilledEllipse,
    Line,
    Arrow,
};

class PreviewView : public QGraphicsView
{
    Q_OBJECT

   public:
    explicit PreviewView(QGraphicsScene* scene, QWidget* parent = nullptr, QUndoStack* undoStack = nullptr);
    void setTool(PreviewTool tool);
    QColor getDrawColor();
    void setDrawColor(QColor color);

   protected:
    // overrides
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

   private:
    // markup
    void _beginStroke(const QPoint& pos);
    void _updateStroke(const QPoint& pos);
    void _commitStroke();
    void _beginShape(const QPoint& pos);
    void _updateShape(const QPoint& pos);
    void _commitShape();
    // markup helpers
    QGraphicsItem* _makeArrowItem(const QPointF& from, const QPointF& to, const QPen& pen);
    static QPainterPath _computeArrowPath(const QPointF& from, const QPointF& to);
    QGraphicsItem* _createShapeItem(const QRectF& rect);
    void _applyShapeGeometry(QGraphicsItem* item, const QRectF& rect);

    std::function<void(qreal)> _onZoom;
    QUndoStack* _undoStack;
    PreviewTool _tool;
    QColor _drawColor;
    // Free Draw
    QGraphicsPathItem* _currentStroke;
    QPainterPath _currentPath;
    // Shapes
    QGraphicsItem* _currentItem;
    QPointF _shapeStart;
    QPointF _shapeEnd;
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

    void _save();
    void _copy();
    void _resetView();
    void _resetSize();
    QPixmap _renderScene();
};