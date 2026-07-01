#pragma once
#include <QWidget>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <windows.h>

class NativeKeyEdit : public QKeySequenceEdit
{
    Q_OBJECT
   public:
    explicit NativeKeyEdit(QWidget* parent = nullptr) : QKeySequenceEdit(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setEnabled(false);
    }

    void beginCapture()
    {
        _capturing = true;
        setEnabled(true);
        setFocus();
    }

    void endCapture()
    {
        _capturing = false;
        setEnabled(false);
        clearFocus();
    }

    bool hasNewKey() const { return _vk != 0; }
    UINT nativeVk() const { return _vk; }
    UINT nativeModifiers() const { return _modifiers; }
    bool isCapturing() const { return _capturing; }

   protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        qDebug() << event->nativeVirtualKey();
        // Only end capture when the last non-mod is released
        switch (event->key())
        {
            case Qt::Key_Control:
            case Qt::Key_Shift:
            case Qt::Key_Alt:
            case Qt::Key_Meta: QKeySequenceEdit::keyPressEvent(event); return;
            default: break;
        }

        QKeySequenceEdit::keyPressEvent(event);
        _vk = static_cast<UINT>(event->nativeVirtualKey());
        _modifiers = MOD_NOREPEAT;
        Qt::KeyboardModifiers qtMods = event->modifiers();
        if (qtMods & Qt::ControlModifier) _modifiers |= MOD_CONTROL;
        if (qtMods & Qt::AltModifier) _modifiers |= MOD_ALT;
        if (qtMods & Qt::ShiftModifier) _modifiers |= MOD_SHIFT;
        if (qtMods & Qt::MetaModifier) _modifiers |= MOD_WIN;
        endCapture();
    }

   private:
    UINT _vk = 0;
    UINT _modifiers = MOD_NOREPEAT;
    bool _capturing = false;
};

class SettingsWindow : public QWidget
{
    Q_OBJECT
   public:
    explicit SettingsWindow(QWidget* parent = nullptr);

   signals:
    void hotkeyChanged(quint32 modifiers, quint32 vk);

   private:
    NativeKeyEdit* _hotkeyEdit;
    QPushButton* _saveButton;
    QKeySequence _lastSaved;

    void _save();
    void _loadSettings();
    void _updateSaveButtonState();
};