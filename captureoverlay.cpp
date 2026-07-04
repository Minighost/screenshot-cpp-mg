#include "captureoverlay.h"
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScreen>
#include "preview.h"
#include "utils.h"

static const int HANDLE_DRAW_SIZE = 2;
static const int HANDLE_HIT_SIZE = 5;

// clang-format off
static const QMap<DragState, Qt::CursorShape> HANDLE_CURSORS = {
    { HANDLE_TL,   Qt::SizeFDiagCursor },
    { HANDLE_TR,   Qt::SizeBDiagCursor },
    { HANDLE_BR,   Qt::SizeFDiagCursor },
    { HANDLE_BL,   Qt::SizeBDiagCursor },
    { HANDLE_T,    Qt::SizeVerCursor   },
    { HANDLE_B,    Qt::SizeVerCursor   },
    { HANDLE_L,    Qt::SizeHorCursor   },
    { HANDLE_R,    Qt::SizeHorCursor   },
    { DRAG_MOVING, Qt::SizeAllCursor   },
    { DRAG_NONE,   Qt::ArrowCursor     },
};
// clang-format on

CaptureOverlay::CaptureOverlay(QWidget* parent)
    : QWidget(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool),
      _dpr(1.0),
      _state(DRAG_NONE),
      _label(new QLabel(this)),
      _saveButton(new QPushButton(tintedIcon(":/svgs/save.svg"), "", this)),
      _copyButton(new QPushButton(tintedIcon(":/svgs/copy.svg"), "", this)),
      _cancelButton(new QPushButton(tintedIcon(":/svgs/cancel.svg"), "", this)),
      _actionsBox(new QWidget(this))
{
    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);

    _label->setStyleSheet(
        "color: white; background: rgba(0,0,0,160);"
        "padding: 3px 6px; border-radius: 4px; font-size: 11px;"
    );
    _label->hide();

    _saveButton->setToolTip("Save (Ctrl+S)");
    _copyButton->setToolTip("Copy (Ctrl+C)");
    _cancelButton->setToolTip("Cancel (Esc)");

    for (QPushButton* btn : {_saveButton, _copyButton, _cancelButton})
    {
        btn->setMouseTracking(true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setIconSize(QSize(14, 14));
    }

    connect(_saveButton, &QPushButton::clicked, this, &CaptureOverlay::saveSelection);
    connect(_copyButton, &QPushButton::clicked, this, &CaptureOverlay::copyToClipboard);
    connect(_cancelButton, &QPushButton::clicked, this, &CaptureOverlay::cleanClose);

    QHBoxLayout* actionsLayout = new QHBoxLayout(_actionsBox);
    actionsLayout->addWidget(_saveButton);
    actionsLayout->addWidget(_copyButton);
    actionsLayout->addWidget(_cancelButton);
    actionsLayout->setContentsMargins(1, 1, 1, 1);
    actionsLayout->setSpacing(4);

    _actionsBox->setStyleSheet(R"(
        QWidget {
            background: rgba(0, 0, 0, 160);
            border-radius: 4px;
            padding: 2px;
        }
        QPushButton {
            background: transparent;
            width: 14px;
            height: 14px;
            border-style: solid;
            border-width: 1px;
            border-radius: 2px;
            border-color: rgba(0, 0, 0, 0);
            padding: 0px;
        }
        QPushButton:hover {
            border-color: rgba(255, 255, 255, 255);
        }
    )");
    _actionsBox->setMouseTracking(true);
    _actionsBox->setFocusPolicy(Qt::NoFocus);
    _actionsBox->adjustSize();
    _actionsBox->hide();

    _label->setFocusPolicy(Qt::NoFocus);
}

void CaptureOverlay::show()
{
    _sel = QRect();
    _state = DRAG_NONE;
    _label->hide();
    _actionsBox->hide();

    QScreen* screen = QGuiApplication::primaryScreen();
    _dpr = screen->devicePixelRatio();

    QRect virtualGeo = screen->virtualGeometry();
    _screenshot = screen->grabWindow(0, virtualGeo.x(), virtualGeo.y(), virtualGeo.width(), virtualGeo.height());

    _screenshotLogical =
        _screenshot.scaled(virtualGeo.width(), virtualGeo.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    setGeometry(virtualGeo);
    QWidget::show();
    activateWindow();
    setFocus();
}

void CaptureOverlay::cleanClose() { close(); }

void CaptureOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (!_screenshotLogical.isNull()) painter.drawPixmap(rect(), _screenshotLogical);

    painter.fillRect(rect(), QColor(0, 0, 0, 100));

    if (_sel.isNull() || !_sel.isValid()) return;

    QRect sel = _sel.normalized();

    if (!_screenshot.isNull())
    {
        QRect phys = physicalCrop(sel, _dpr);
        painter.drawPixmap(sel, _screenshot, phys);
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QColor(0, 120, 215, 230));
    painter.drawRect(sel);

    painter.setPen(QColor(255, 255, 255, 230));
    painter.setBrush(QColor(0, 120, 215, 200));
    for (const QRect& hr : handlesFor(sel, HANDLE_DRAW_SIZE)) painter.drawRect(hr);
}

void CaptureOverlay::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        cleanClose();
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    QPoint pos = event->pos();

    if (!_sel.isNull() && _sel.isValid())
    {
        QRect sel = _sel.normalized();

        const auto handles = handlesFor(sel, HANDLE_HIT_SIZE);
        for (auto it = handles.begin(); it != handles.end(); ++it)
        {
            if (it.value().contains(pos))
            {
                _state = it.key();
                _dragStart = pos;
                _selAtDrag = sel;
                return;
            }
        }

        if (sel.contains(pos))
        {
            _state = DRAG_MOVING;
            _dragStart = pos;
            _selAtDrag = sel;
            return;
        }
    }

    _sel = QRect(pos, pos);
    _dragStart = pos;
    _selAtDrag = QRect(pos, pos);
    _state = HANDLE_BR;
}

void CaptureOverlay::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();

    if (_state == DRAG_NONE)
    {
        updateCursor(pos);
        return;
    }

    QPoint delta = pos - _dragStart;

    if (_state == DRAG_MOVING) { _sel = QRect(_selAtDrag.topLeft() + delta, _selAtDrag.size()); }
    else
    {
        _sel = applyHandleDrag(_selAtDrag, _state, delta);
    }

    updateCursor(pos);
    updateLabelAndActions();
    update();
}

void CaptureOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;

    if (_sel.width() < 5 || _sel.height() < 5)
    {
        _sel = QRect();
        _label->hide();
        _actionsBox->hide();
    }

    _sel = _sel.normalized();
    _state = DRAG_NONE;
    updateCursor(event->pos());
    updateLabelAndActions();
    update();
}

void CaptureOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) { cleanClose(); }
    else if (event->key() == Qt::Key_S && event->modifiers() == Qt::ControlModifier) { saveSelection(); }
    else if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) { copyToClipboard(); }
    else if (event->key() == Qt::Key_C && event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        openPreview();
    }
}

QRect CaptureOverlay::handleRect(int cx, int cy, int size) const
{ return QRect(cx - size, cy - size, size * 2, size * 2); }

QMap<DragState, QRect> CaptureOverlay::handlesFor(const QRect& sel, int size) const
{
    int x = sel.left(), y = sel.top(), r = sel.right(), b = sel.bottom();
    int mx = (x + r) / 2, my = (y + b) / 2;
    return {
        {HANDLE_TL, handleRect(x, y, size)}, {HANDLE_T, handleRect(mx, y, size)}, {HANDLE_TR, handleRect(r, y, size)},
        {HANDLE_R, handleRect(r, my, size)}, {HANDLE_BR, handleRect(r, b, size)}, {HANDLE_B, handleRect(mx, b, size)},
        {HANDLE_BL, handleRect(x, b, size)}, {HANDLE_L, handleRect(x, my, size)},
    };
}

QRect CaptureOverlay::applyHandleDrag(const QRect& sel, DragState handle, const QPoint& delta) const
{
    int l = sel.left(), t = sel.top(), r = sel.right(), b = sel.bottom();
    if (handle == HANDLE_TL || handle == HANDLE_T || handle == HANDLE_TR) t += delta.y();
    if (handle == HANDLE_BL || handle == HANDLE_B || handle == HANDLE_BR) b += delta.y();
    if (handle == HANDLE_TL || handle == HANDLE_L || handle == HANDLE_BL) l += delta.x();
    if (handle == HANDLE_TR || handle == HANDLE_R || handle == HANDLE_BR) r += delta.x();
    return QRect(QPoint(l, t), QPoint(r, b)).normalized();
}

void CaptureOverlay::updateCursor(const QPoint& pos)
{
    if (!_sel.isNull() && _sel.isValid())
    {
        QRect sel = _sel.normalized();
        const auto handles = handlesFor(sel, HANDLE_HIT_SIZE);
        for (auto it = handles.begin(); it != handles.end(); ++it)
        {
            if (it.value().contains(pos))
            {
                setCursor(HANDLE_CURSORS[it.key()]);
                return;
            }
        }
        if (sel.contains(pos))
        {
            setCursor(HANDLE_CURSORS[DRAG_MOVING]);
            return;
        }
    }
    setCursor(HANDLE_CURSORS[DRAG_NONE]);
}

void CaptureOverlay::updateLabelAndActions()
{
    if (_sel.isNull())
    {
        _label->hide();
        _actionsBox->hide();
        return;
    }

    QRect sel = _sel.normalized();

    _label->setText(QString("%1 x %2").arg(sel.width()).arg(sel.height()));
    _label->adjustSize();
    int labelX = qMax(4, sel.x());
    int labelY = qMax(4, sel.y() - _label->height() - 4);
    _label->move(labelX, labelY);
    _label->show();

    _actionsBox->adjustSize();
    int actionsX = qMax(4, sel.x());
    int actionsY = qMin(geometry().height() - _actionsBox->height() - 4, sel.y() + sel.height() + 4);
    _actionsBox->move(actionsX, actionsY);
    _actionsBox->show();
}

void CaptureOverlay::saveSelection()
{
    if (_sel.isNull() || _screenshot.isNull()) return;

    QRect sel = _sel.normalized();
    QPixmap crop = _screenshot.copy(physicalCrop(sel, _dpr));

    savePixmap(crop, this);
    cleanClose();
}

void CaptureOverlay::copyToClipboard()
{
    if (_sel.isNull() || _screenshot.isNull()) return;

    QRect sel = _sel.normalized();
    QPixmap crop = _screenshot.copy(physicalCrop(sel, _dpr));

    copyPixmap(crop);
    cleanClose();
}

void CaptureOverlay::openPreview()
{
    if (_sel.isNull() || _screenshot.isNull()) return;

    QRect sel = _sel.normalized();
    QPixmap crop = _screenshot.copy(physicalCrop(sel, _dpr));

    PreviewWindow* window = new PreviewWindow();
    window->setPixmap(crop);
    window->show();
    cleanClose();
}