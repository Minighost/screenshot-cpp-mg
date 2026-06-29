#include "preview.h"
#include "utils.h"
#include "drawcommands.h"
#include <QCloseEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QStyle>
#include <QPen>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QActionGroup>
#include <QColorDialog>
#include <cmath>
#include <QtMath>

// ---
// Preview View
// ---

PreviewView::PreviewView(
    QGraphicsScene* scene, QWidget* parent, std::function<void(qreal)> onZoom, QUndoStack* undoStack
)
    : QGraphicsView(scene, parent),
      _onZoom(onZoom),
      _undoStack(undoStack),
      _tool(PreviewTool::Pan),
      _drawColor(Qt::red),
      _currentItem(nullptr),
      _shapeStart(),
      _shapeEnd()
{
}

void PreviewView::setTool(PreviewTool tool)
{
    _tool = tool;
    setDragMode(tool == PreviewTool::Pan ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
}

QColor PreviewView::getDrawColor() { return _drawColor; }

void PreviewView::setDrawColor(QColor color) { _drawColor = color; }

void PreviewView::wheelEvent(QWheelEvent* event)
{
    qreal factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    qreal current_scale = this->transform().m11();
    if ((factor < 1 && current_scale > 0.1) || (factor > 1 && current_scale < 20))
    {
        this->scale(factor, factor);
        if (_onZoom) _onZoom(transform().m11());
    }
}

void PreviewView::mousePressEvent(QMouseEvent* event)
{
    if (_tool == PreviewTool::Pan || event->button() != Qt::LeftButton)
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    if (_tool == PreviewTool::FreeDraw)
        _beginStroke(event->pos());
    else
        _beginShape(event->pos());
}

void PreviewView::mouseMoveEvent(QMouseEvent* event)
{
    if (_tool == PreviewTool::Pan || !_currentItem && !_currentStroke)
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if (_tool == PreviewTool::FreeDraw)
        _updateStroke(event->pos());
    else
        _updateShape(event->pos());
}

void PreviewView::mouseReleaseEvent(QMouseEvent* event)
{
    if (_tool == PreviewTool::Pan || event->button() != Qt::LeftButton)
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    if (_tool == PreviewTool::FreeDraw)
        _commitStroke();
    else
        _commitShape();
}

void PreviewView::_beginStroke(const QPoint& pos)
{
    _currentPath = QPainterPath();
    _currentPath.moveTo(mapToScene(pos));
    _currentStroke = new QGraphicsPathItem();
    _currentStroke->setPen(QPen(_drawColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    _currentStroke->setPath(_currentPath);
    scene()->addItem(_currentStroke);
}

void PreviewView::_updateStroke(const QPoint& pos)
{
    if (!_currentStroke) return;
    _currentPath.lineTo(mapToScene(pos));
    _currentStroke->setPath(_currentPath);
}

void PreviewView::_commitStroke()
{
    if (!_currentStroke) return;
    _undoStack->push(new AddItemCommand(scene(), _currentStroke));
    _currentStroke = nullptr;
}

void PreviewView::_beginShape(const QPoint& pos)
{
    _shapeStart = mapToScene(pos);
    _shapeEnd = _shapeStart;
    _currentItem = _createShapeItem(QRectF(_shapeStart, _shapeStart));
    if (_currentItem) scene()->addItem(_currentItem);
}

void PreviewView::_updateShape(const QPoint& pos)
{
    if (!_currentItem) return;
    _shapeEnd = mapToScene(pos);
    QRectF rect = QRectF(_shapeStart, _shapeEnd).normalized();
    _applyShapeGeometry(_currentItem, rect);
    scene()->update();
}

void PreviewView::_commitShape()
{
    if (!_currentItem) return;
    _undoStack->push(new AddItemCommand(scene(), _currentItem));
    _currentItem = nullptr;
}

QGraphicsItem* PreviewView::_makeArrowItem(const QPointF& from, const QPointF& to, const QPen& pen)
{
    auto* item = new QGraphicsPathItem(_computeArrowPath(from, to));
    item->setPen(pen);
    item->setBrush(pen.color());
    return item;
}

QPainterPath PreviewView::_computeArrowPath(const QPointF& from, const QPointF& to)
{
    QPainterPath path;
    path.moveTo(from);
    path.lineTo(to);

    QLineF line(from, to);
    if (line.length() < 1.0) return path;  // avoid degenerate arrow

    double angle = qDegreesToRadians(line.angle());
    double arrowSize = 12.0;

    QPointF p1 = to + QPointF(cos(angle + M_PI - M_PI / 6) * arrowSize, -sin(angle + M_PI - M_PI / 6) * arrowSize);
    QPointF p2 = to + QPointF(cos(angle + M_PI + M_PI / 6) * arrowSize, -sin(angle + M_PI + M_PI / 6) * arrowSize);

    path.moveTo(to);
    path.lineTo(p1);
    path.lineTo(p2);
    path.closeSubpath();

    return path;
}

QGraphicsItem* PreviewView::_createShapeItem(const QRectF& rect)
{
    QPen pen(_drawColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush fillBrush(_drawColor);
    QBrush noFill(Qt::NoBrush);

    switch (_tool)
    {
        case PreviewTool::Rectangle:
        {
            auto* item = new QGraphicsRectItem(rect);
            item->setPen(pen);
            item->setBrush(noFill);
            return item;
        }
        case PreviewTool::FilledRectangle:
        {
            auto* item = new QGraphicsRectItem(rect);
            item->setPen(pen);
            item->setBrush(fillBrush);
            return item;
        }
        case PreviewTool::Ellipse:
        {
            auto* item = new QGraphicsEllipseItem(rect);
            item->setPen(pen);
            item->setBrush(noFill);
            return item;
        }
        case PreviewTool::FilledEllipse:
        {
            auto* item = new QGraphicsEllipseItem(rect);
            item->setPen(pen);
            item->setBrush(fillBrush);
            return item;
        }
        case PreviewTool::Line:
        {
            auto* item = new QGraphicsLineItem(QLineF(_shapeStart, _shapeStart));
            item->setPen(pen);
            return item;
        }
        case PreviewTool::Arrow:
        {
            auto* item = _makeArrowItem(_shapeStart, _shapeEnd, pen);
            return item;
        }
        default: return nullptr;
    }
}

void PreviewView::_applyShapeGeometry(QGraphicsItem* item, const QRectF& rect)
{
    switch (_tool)
    {
        case PreviewTool::Rectangle:
        case PreviewTool::FilledRectangle: static_cast<QGraphicsRectItem*>(item)->setRect(rect); break;
        case PreviewTool::Ellipse:
        case PreviewTool::FilledEllipse: static_cast<QGraphicsEllipseItem*>(item)->setRect(rect); break;
        case PreviewTool::Line: static_cast<QGraphicsLineItem*>(item)->setLine(QLineF(_shapeStart, _shapeEnd)); break;
        case PreviewTool::Arrow:
            static_cast<QGraphicsPathItem*>(item)->setPath(_computeArrowPath(_shapeStart, _shapeEnd));
            break;
        default: break;
    }
}

// ---
// Preview Window
// ---

PreviewWindow::PreviewWindow()
    : QWidget(),
      _scene(new QGraphicsScene(this)),
      _pixmapItem(_scene->addPixmap(QPixmap())),
      _undoStack(new QUndoStack(this)),
      _view(new PreviewView(
          _scene, this, [this](qreal scale) { _updateZoomLabel(scale); }, _undoStack
      )),
      _toolbar(new QToolBar(this)),
      _zoomLabel(new QLabel("100%", this))
{
    setWindowTitle("Preview");
    setWindowIcon(QIcon(":/screenshot-mg.ico"));
    setAttribute(Qt::WA_DeleteOnClose);

    _view->setRenderHint(QPainter::Antialiasing);
    _view->setDragMode(QGraphicsView::ScrollHandDrag);
    _view->setFrameShape(QFrame::NoFrame);
    _view->setFocusPolicy(Qt::NoFocus);
    _view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Spacer for some extra padding on the left side
    QWidget* spacer = new QWidget(this);
    spacer->setFixedWidth(2);
    _toolbar->addWidget(spacer);

    QAction* saveAction = _toolbar->addAction(tintedIcon(":/svgs/save.svg"), "");
    QAction* copyAction = _toolbar->addAction(tintedIcon(":/svgs/copy.svg"), "");
    QAction* resetZoomAction = _toolbar->addAction(tintedIcon(":/svgs/reset-zoom-level.svg"), "");
    QAction* resetWindowAction = _toolbar->addAction(tintedIcon(":/svgs/reset-window-size.svg"), "");

    saveAction->setToolTip("Save (Ctrl+S)");
    copyAction->setToolTip("Copy (Ctrl+C)");
    resetZoomAction->setToolTip("Reset zoom (R)");
    resetWindowAction->setToolTip("Reset window (Shift+R)");

    connect(saveAction, &QAction::triggered, this, &PreviewWindow::_save);
    connect(copyAction, &QAction::triggered, this, &PreviewWindow::_copy);
    connect(resetZoomAction, &QAction::triggered, this, &PreviewWindow::_resetView);
    connect(resetWindowAction, &QAction::triggered, this, &PreviewWindow::_resetSize);

    _toolbar->addWidget(_zoomLabel);
    _toolbar->addSeparator();

    // Add a color picker (not part of the action group)
    QAction* colorSelect = _toolbar->addAction(colorSwatchIcon(Qt::red), "");
    colorSelect->setToolTip("Select a drawing color");
    connect(
        colorSelect, &QAction::triggered, this,
        [this, colorSelect]()
        {
            QColor color = QColorDialog::getColor(_view->getDrawColor(), this, "Pick Color");
            if (color.isValid())
            {
                _view->setDrawColor(color);
                colorSelect->setIcon(colorSwatchIcon(color));
            }
        }
    );

    // Drawing stuff
    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    auto addTool = [&](const QIcon& icon, const QString& tooltip, PreviewTool tool, bool checked = false)
    {
        QAction* action = _toolbar->addAction(icon, "");
        action->setCheckable(true);
        action->setChecked(checked);
        action->setToolTip(tooltip);
        toolGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, tool]() { _view->setTool(tool); });
        return action;
    };

    addTool(tintedIcon(":/svgs/pan.svg"), "Pan", PreviewTool::Pan, true);
    addTool(tintedIcon(":/svgs/draw.svg"), "Free Draw", PreviewTool::FreeDraw);
    addTool(tintedIcon(":/svgs/rect-outline.svg"), "Rectangle", PreviewTool::Rectangle);
    addTool(tintedIcon(":/svgs/rect-fill.svg"), "Filled Rectangle", PreviewTool::FilledRectangle);
    addTool(tintedIcon(":/svgs/elli-outline.svg"), "Ellipse", PreviewTool::Ellipse);
    addTool(tintedIcon(":/svgs/elli-fill.svg"), "Filled Ellipse", PreviewTool::FilledEllipse);
    addTool(tintedIcon(":/svgs/line.svg"), "Line", PreviewTool::Line);
    addTool(tintedIcon(":/svgs/arrow.svg"), "Arrow", PreviewTool::Arrow);

    _toolbar->setIconSize(QSize(16, 16));
    _toolbar->setStyleSheet(
        "QToolBar { padding: 4px; spacing: 4px; background: rgb(25, 35, 46) }"
        "QToolButton { width: 24px; height: 24px; background: rgb(46, 46, 46); border-radius: 4px; }"
        "QToolButton:hover { background: rgb(84, 84, 84); }"
        "QToolButton:checked { background: rgb(84, 84, 84); }"
        "QLabel { padding: 4px; font-size: 16px }"
    );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_toolbar);
    layout->addWidget(_view);
    setLayout(layout);
}

void PreviewWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_S && event->modifiers() == Qt::ControlModifier) { _save(); }
    else if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) { _copy(); }
    else if (event->key() == Qt::Key_R && event->modifiers() == Qt::NoModifier) { _resetView(); }
    else if (event->key() == Qt::Key_R && event->modifiers() == Qt::ShiftModifier) { _resetSize(); }
    else if (event->key() == Qt::Key_Z && event->modifiers() == Qt::ControlModifier) { _undoStack->undo(); }
    else if (event->key() == Qt::Key_Y && event->modifiers() == Qt::ControlModifier) { _undoStack->redo(); }
}

void PreviewWindow::setPixmap(const QPixmap& pixmap)
{
    _pixmapItem->setPixmap(pixmap);
    QRectF padded = _pixmapItem->boundingRect().adjusted(-1000, -1000, 1000, 1000);
    _scene->setSceneRect(padded);
    _view->resetTransform();
    _view->centerOn(_pixmapItem);
    resize(pixmap.width(), pixmap.height());
}

void PreviewWindow::_save() { savePixmap(_renderScene(), this); }

void PreviewWindow::_copy() { copyPixmap(_renderScene()); }

void PreviewWindow::_resetView()
{
    _view->resetTransform();
    _view->centerOn(_pixmapItem);
    _updateZoomLabel(1);
}

void PreviewWindow::_resetSize()
{
    const QPixmap& pixmap = _pixmapItem->pixmap();
    resize(pixmap.width(), pixmap.height());
}

void PreviewWindow::closeEvent(QCloseEvent* event)
{
    _undoStack->clear();
    QWidget::closeEvent(event);
}

void PreviewWindow::_updateZoomLabel(qreal scale) { _zoomLabel->setText(QString("%1%").arg(qRound(scale * 100))); }

QPixmap PreviewWindow::_renderScene()
{
    QPixmap result(_pixmapItem->pixmap().size());
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    _scene->render(&painter, QRectF(), _pixmapItem->boundingRect());
    return result;
}