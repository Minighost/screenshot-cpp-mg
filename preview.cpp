#include "preview.h"
#include "utils.h"
#include <QWheelEvent>
#include <QVBoxLayout>

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
      _pixmap_item(_scene->addPixmap(QPixmap())),
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

    connect(_toolbar->addAction("Save"), &QAction::triggered, this, &PreviewWindow::_save);
    connect(_toolbar->addAction("Copy"), &QAction::triggered, this, &PreviewWindow::_copy);
    connect(_toolbar->addAction("Reset"), &QAction::triggered, this, &PreviewWindow::_resetView);
    _toolbar->addWidget(_zoomLabel);

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
    else if (event->key() == Qt::Key_R) { _resetView(); }
}

void PreviewWindow::setPixmap(const QPixmap& pixmap)
{
    _pixmap_item->setPixmap(pixmap);
    _scene->setSceneRect(_pixmap_item->boundingRect());
    _view->resetTransform();
    _view->setBaseSize(pixmap.width(), pixmap.height());
    adjustSize();
}

void PreviewWindow::_save() { savePixmap(_pixmap_item->pixmap(), this); }

void PreviewWindow::_copy() { copyPixmap(_pixmap_item->pixmap()); }

void PreviewWindow::_resetView()
{
    _view->resetTransform();
    _view->centerOn(_pixmap_item);
    _updateZoomLabel(1);
}

void PreviewWindow::_updateZoomLabel(qreal scale) { _zoomLabel->setText(QString("%1%").arg(qRound(scale * 100))); }