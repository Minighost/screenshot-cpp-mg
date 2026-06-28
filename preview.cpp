#include "preview.h"
#include "utils.h"
#include "drawcommands.h"
#include <QCloseEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QStyle>
#include <QPen>

PreviewView::PreviewView(
    QGraphicsScene* scene, QWidget* parent, std::function<void(qreal)> onZoom, QUndoStack* undoStack
)
    : QGraphicsView(scene, parent),
      _onZoom(onZoom),
      _undoStack(undoStack),
      _tool(PreviewTool::Pan),
      _currentStroke(nullptr)
{
}

void PreviewView::setTool(PreviewTool tool)
{
    _tool = tool;
    setDragMode(tool == PreviewTool::Pan ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
}

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

    _currentPath = QPainterPath();
    _currentPath.moveTo(mapToScene(event->pos()));

    _currentStroke = new QGraphicsPathItem();
    _currentStroke->setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    _currentStroke->setPath(_currentPath);
    scene()->addItem(_currentStroke);
}

void PreviewView::mouseMoveEvent(QMouseEvent* event)
{
    if (_tool == PreviewTool::Pan || !_currentStroke)
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    _currentPath.lineTo(mapToScene(event->pos()));
    _currentStroke->setPath(_currentPath);
}

void PreviewView::mouseReleaseEvent(QMouseEvent* event)
{
    if (_tool == PreviewTool::Pan || !_currentStroke)
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    _undoStack->push(new StrokeCommand(scene(), _currentStroke));
    _currentStroke = nullptr;
}

PreviewWindow::PreviewWindow()
    : QWidget(),
      _scene(new QGraphicsScene(this)),
      _pixmapItem(_scene->addPixmap(QPixmap())),
      _undoStack(new QUndoStack(this)),
      _view(new PreviewView(
          _scene, this, [this](qreal scale) { _updateZoomLabel(scale); }, _undoStack
      )),
      _toolbar(new QToolBar(this)),
      _zoomLabel(new QLabel("100%", this)),
      _drawAction(nullptr)
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

    QAction* saveAction = _toolbar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "");
    QAction* copyAction = _toolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon), "");
    QAction* resetZoomAction = _toolbar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), "");
    QAction* resetWindowAction = _toolbar->addAction(style()->standardIcon(QStyle::SP_TitleBarMaxButton), "");

    saveAction->setToolTip("Save (Ctrl+S)");
    copyAction->setToolTip("Copy (Ctrl+C)");
    resetZoomAction->setToolTip("Reset zoom (R)");
    resetWindowAction->setToolTip("Reset window (Shift+R)");

    connect(saveAction, &QAction::triggered, this, &PreviewWindow::_save);
    connect(copyAction, &QAction::triggered, this, &PreviewWindow::_copy);
    connect(resetZoomAction, &QAction::triggered, this, &PreviewWindow::_resetView);
    connect(resetWindowAction, &QAction::triggered, this, &PreviewWindow::_resetSize);

    _toolbar->addWidget(_zoomLabel);

    // Drawing stuff
    _drawAction = _toolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), "");
    _drawAction->setToolTip("Draw (D)");
    _drawAction->setCheckable(true);
    connect(
        _drawAction, &QAction::toggled, this,
        [this](bool checked) { _view->setTool(checked ? PreviewTool::FreeDraw : PreviewTool::Pan); }
    );

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
    else if (event->key() == Qt::Key_D && event->modifiers() == Qt::NoModifier) { _drawAction->toggle(); }
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