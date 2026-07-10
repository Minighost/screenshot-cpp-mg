#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QCheckBox>
#include <QComboBox>
#include "types.h"
#include "keycapturemanager.h"

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
    QPushButton* _saveButton;
    QPushButton* _cancelButton;
    bool _isCapturing = false;
    HotkeyId _capturingId;
    static const QMap<HotkeyId, HotkeyData> DEFAULT_HOTKEYS;
    QLabel* _statusLabel;

    QCheckBox* _nonPersistent;
    QComboBox* _regionAction;
    QComboBox* _fullscreenAction;
    QComboBox* _windowAction;
    QLineEdit* _savePath;

    HotkeyRow _makeHotkeyRow(HotkeyId id);
    void _beginCapture(HotkeyId id);
    void _endCapture(bool revert);
    void _setCurrent(HotkeyId id, const HotkeyData& data);
    void _save();
    void _loadSettings();
    void _updateStatusLabel();

    KeyCaptureManager* _keyCaptureManager;
};