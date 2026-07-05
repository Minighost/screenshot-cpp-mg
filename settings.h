#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QCheckBox>
#include <windows.h>

struct HotkeyData
{
    UINT vk = 0;
    UINT modifiers = MOD_NOREPEAT;
    bool isEmpty() const { return vk == 0; }
    bool operator==(const HotkeyData& o) const { return vk == o.vk && modifiers == o.modifiers; }
    bool operator!=(const HotkeyData& o) const { return !(*this == o); }
};

enum HotkeyId : quint32
{
    Overlay = 1,
    Fullscreen = 2,
    WindowCapture = 3
};

struct HotkeyRow
{
    QLabel* label = nullptr;
    QPushButton* changeButton = nullptr;
    QPushButton* clearButton = nullptr;
    QPushButton* revertButton = nullptr;
};

class SettingsWindow : public QWidget
{
    Q_OBJECT
   public:
    explicit SettingsWindow(QWidget* parent = nullptr);

   signals:
    void hotkeyChanged(HotkeyId id, quint32 modifiers, quint32 vk);

   protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void closeEvent(QCloseEvent* event) override;

   private:
    QMap<HotkeyId, HotkeyRow> _rows;
    QMap<HotkeyId, HotkeyData> _current;
    QMap<HotkeyId, HotkeyData> _lastSaved;
    QPushButton* _saveButton;
    bool _isCapturing = false;
    HotkeyId _capturingId;
    static const QMap<HotkeyId, HotkeyData> DEFAULT_HOTKEYS;
    QCheckBox* _fullscreenPreview;
    QCheckBox* _windowPreview;
    bool _lastSavedFullscreenPreview = false;
    bool _lastSavedWindowPreview = false;
    QLabel* _statusLabel;


    HotkeyRow _makeRow(HotkeyId id);
    void _beginCapture(HotkeyId id);
    void _endCapture(bool revert);
    void _setCurrent(HotkeyId id, const HotkeyData& data);
    void _save();
    void _loadSettings();
    void _updateStatusLabel();
    void _registerRawInput();
    void _unregisterRawInput();
};