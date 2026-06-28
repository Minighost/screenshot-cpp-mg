#include "preview.h"
#include "utils.h"
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QStyle>

PreviewView::PreviewView(QGraphicsScene* scene, QWidget* parent, std::function<void(qreal)> onZoom)
    : QGraphicsView(scene, parent), _onZoom(onZoom)
{
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

PreviewWindow::PreviewWindow()
    : QWidget(),
      _scene(new QGraphicsScene(this)),
      _pixmapItem(_scene->addPixmap(QPixmap())),
      _view(new PreviewView(_scene, this, [this](qreal scale) { _updateZoomLabel(scale); })),
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
    _toolbar->setIconSize(QSize(16, 16));
    _toolbar->setStyleSheet(
        "QToolBar { padding: 4px; spacing: 4px; background: rgb(25, 35, 46) }"
        "QToolButton { width: 24px; height: 24px; background: rgb(46, 46, 46); border-radius: 4px; }"
        "QToolButton:hover { background: rgb(84, 84, 84); }"
        "QLabel { padding: 4px; font-size: 16px }"
    );
    _toolbar->adjustSize();

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

void PreviewWindow::_save() { savePixmap(_pixmapItem->pixmap(), this); }

void PreviewWindow::_copy() { copyPixmap(_pixmapItem->pixmap()); }

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

void PreviewWindow::_updateZoomLabel(qreal scale) { _zoomLabel->setText(QString("%1%").arg(qRound(scale * 100))); }