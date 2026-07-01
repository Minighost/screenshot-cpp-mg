#pragma once
#include <QWidget>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QLabel>
#include <windows.h>

class NativeKeyEdit : public QKeySequenceEdit
{
    Q_OBJECT
   public:
    explicit NativeKeyEdit(QWidget* parent = nullptr) : QKeySequenceEdit(parent) {}

    UINT nativeVk() const { return _vk; }
    UINT nativeModifiers() const { return _modifiers; }
    bool hasNewKey() const { return _vk != 0; }

   protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        QKeySequenceEdit::keyPressEvent(event);  // let Qt update the displayed sequence

        _vk = static_cast<UINT>(event->nativeVirtualKey());

        _modifiers = MOD_NOREPEAT;
        Qt::KeyboardModifiers qtMods = event->modifiers();
        if (qtMods & Qt::ControlModifier) _modifiers |= MOD_CONTROL;
        if (qtMods & Qt::AltModifier) _modifiers |= MOD_ALT;
        if (qtMods & Qt::ShiftModifier) _modifiers |= MOD_SHIFT;
        if (qtMods & Qt::MetaModifier) _modifiers |= MOD_WIN;
    }

   private:
    UINT _vk = 0;
    UINT _modifiers = MOD_NOREPEAT;
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