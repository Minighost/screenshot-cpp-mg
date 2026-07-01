#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <windows.h>

struct HotkeyData
{
    UINT vk = 0;
    UINT modifiers = MOD_NOREPEAT;
    bool isEmpty() const { return vk == 0; }
    bool operator==(const HotkeyData& o) const { return vk == o.vk && modifiers == o.modifiers; }
};

class SettingsWindow : public QWidget
{
    Q_OBJECT
   public:
    explicit SettingsWindow(QWidget* parent = nullptr);

   signals:
    void hotkeyChanged(quint32 modifiers, quint32 vk);

   protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void closeEvent(QCloseEvent* event) override;

   private:
    QLabel* _hotkeyLabel;
    QPushButton* _saveButton;
    QPushButton* _changeButton;
    QPushButton* _clearButton;

    HotkeyData _current;    // what's currently shown/captured
    HotkeyData _lastSaved;  // what's saved to disk

    bool _isCapturing = false;

    void _save();
    void _loadSettings();
    void _updateSaveButtonState();
    void _registerRawInput();
    void _unregisterRawInput();
    void _beginCapture();
    void _endCapture(bool revert = false);
    void _setCurrent(const HotkeyData& data);
};