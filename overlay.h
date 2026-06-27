#pragma once
#include <QWidget>
#include <QRect>
#include <QPoint>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QList>

class PreviewWindow;

// Which part of the selection the user is dragging
enum DragState
{
    DRAG_NONE,
    DRAG_MOVING,
    HANDLE_TL,
    HANDLE_T,
    HANDLE_TR,
    HANDLE_R,
    HANDLE_BR,
    HANDLE_B,
    HANDLE_BL,
    HANDLE_L,
};

class CaptureOverlay : public QWidget
{
    Q_OBJECT

   public:
    explicit CaptureOverlay(QWidget* parent = nullptr);
    void show();
    void cleanClose();

   protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    QPixmap _screenshot;
    QPixmap _screenshotLogical;
    qreal _dpr;
    DragState _state;
    QRect _sel;
    QPoint _dragStart;
    QRect _selAtDrag;

    QLabel* _label;
    QPushButton* _saveButton;
    QPushButton* _copyButton;
    QPushButton* _cancelButton;
    QWidget* _actionsBox;

    QList<PreviewWindow*> _previewWindows;

    QRect handleRect(int cx, int cy, int size) const;
    QMap<DragState, QRect> handlesFor(const QRect& sel, int size) const;
    QRect applyHandleDrag(const QRect& sel, DragState handle, const QPoint& delta) const;
    QRect cropPhys(const QRect& sel) const;

    void updateCursor(const QPoint& pos);
    void updateLabelAndActions();

    void saveSelection();
    void copyToClipboard();
    void openPreview();
};