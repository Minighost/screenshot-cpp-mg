#include "settings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QCloseEvent>
#include <QCoreApplication>
// #include <QDebug>
#include "utils.h"

const QMap<HotkeyId, HotkeyData> SettingsWindow::DEFAULT_HOTKEYS = {
    {HotkeyId::Overlay, {VK_SNAPSHOT, MOD_NOREPEAT}},
    {HotkeyId::Fullscreen, {VK_SNAPSHOT, MOD_SHIFT | MOD_NOREPEAT}},
    {HotkeyId::WindowCapture, {VK_SNAPSHOT, MOD_ALT | MOD_NOREPEAT}},
};

SettingsWindow::SettingsWindow(QWidget* parent) : QWidget(parent), _saveButton(new QPushButton("Save", this))
{
    setWindowTitle("Settings");
    setAttribute(Qt::WA_DeleteOnClose);

    QLabel* keybind_notice = new QLabel("Keybinds are disabled while this window is open!", this);
    keybind_notice->setStyleSheet("color: rgba(255, 255, 0, 255);");

    // actual layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(keybind_notice);
    layout->addSpacing(5);

    for (HotkeyId id : {HotkeyId::Overlay, HotkeyId::Fullscreen, HotkeyId::WindowCapture})
    {
        HotkeyRow row = _makeRow(id);
        _rows[id] = row;

        QString rowLabel;
        if (id == HotkeyId::Overlay)
            rowLabel = "Screenshot:";
        else if (id == HotkeyId::Fullscreen)
            rowLabel = "Fullscreen:";
        else if (id == HotkeyId::WindowCapture)
            rowLabel = "Window:";
        QLabel* curLabel = new QLabel(rowLabel, this);
        curLabel->setFixedWidth(70);

        QHBoxLayout* rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(1);
        rowLayout->addWidget(curLabel);
        rowLayout->addWidget(row.label);
        rowLayout->addStretch();
        rowLayout->addWidget(row.changeButton);
        rowLayout->addWidget(row.clearButton);
        rowLayout->addWidget(row.revertButton);

        layout->addLayout(rowLayout);
    }

    _fullscreenPreview = new QCheckBox("Open preview after fullscreen capture", this);
    _windowPreview = new QCheckBox("Open preview after window capture", this);

    connect(_fullscreenPreview, &QCheckBox::checkStateChanged, this, [this]() { _updateStatusLabel(); });
    connect(_windowPreview, &QCheckBox::checkStateChanged, this, [this]() { _updateStatusLabel(); });

    layout->addWidget(_fullscreenPreview);
    layout->addWidget(_windowPreview);

    layout->addSpacing(5);

    _statusLabel = new QLabel("All settings saved.", this);
    _statusLabel->setStyleSheet("color: rgba(0, 255, 0, 120);");
    layout->addWidget(_statusLabel);

    layout->addSpacing(5);

    connect(_saveButton, &QPushButton::clicked, this, &SettingsWindow::_save);
    layout->addWidget(_saveButton);

    setLayout(layout);
    _loadSettings();
}

HotkeyRow SettingsWindow::_makeRow(HotkeyId id)
{
    HotkeyRow row;
    row.label = new QLabel(this);
    row.changeButton = new QPushButton(tintedIcon(":/svgs/draw.svg", 16), "", this);
    row.clearButton = new QPushButton(tintedIcon(":/svgs/delete.svg", 16), "", this);
    row.revertButton = new QPushButton(tintedIcon(":/svgs/reset-zoom-level.svg", 16), "", this);

    row.changeButton->setFixedSize(24, 24);
    row.clearButton->setFixedSize(24, 24);
    row.revertButton->setFixedSize(24, 24);

    row.changeButton->setIconSize(QSize(12, 12));
    row.clearButton->setIconSize(QSize(12, 12));
    row.revertButton->setIconSize(QSize(12, 12));

    connect(
        row.changeButton, &QPushButton::clicked, this,
        [this, id]()
        {
            if (_isCapturing && _capturingId == id)
                _endCapture(true);
            else
                _beginCapture(id);
        }
    );

    connect(
        row.clearButton, &QPushButton::clicked, this,
        [this, id]()
        {
            if (_isCapturing && _capturingId == id) _endCapture(false);
            _setCurrent(id, {});
        }
    );

    connect(
        row.revertButton, &QPushButton::clicked, this,
        [this, id]()
        {
            if (_isCapturing && _capturingId == id) _endCapture(false);
            _setCurrent(id, DEFAULT_HOTKEYS[id]);
        }
    );

    return row;
}

void SettingsWindow::_beginCapture(HotkeyId id)
{
    // cancel any existing capture first
    if (_isCapturing) _endCapture(false);

    _isCapturing = true;
    _capturingId = id;
    _rows[id].changeButton->setIcon(tintedIcon(":/svgs/cancel.svg", 16));
    _rows[id].label->setText("Press a key...");
    _registerRawInput();
}

void SettingsWindow::_endCapture(bool revert)
{
    if (!_isCapturing) return;
    _rows[_capturingId].changeButton->setIcon(tintedIcon(":/svgs/draw.svg", 16));
    _unregisterRawInput();
    if (revert) _setCurrent(_capturingId, _current[_capturingId]);
    _isCapturing = false;
}

void SettingsWindow::_setCurrent(HotkeyId id, const HotkeyData& data)
{
    _current[id] = data;
    if (data.isEmpty())
        _rows[id].label->setText("(none)");
    else
        _rows[id].label->setText(hotkeyToDisplayString(data.vk, data.modifiers));
    _updateStatusLabel();
}

void SettingsWindow::_save()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    if (_isCapturing) { _endCapture(true); }

    for (auto it = _current.begin(); it != _current.end(); ++it)
    {
        QString key;
        if (it.key() == HotkeyId::Overlay)
            key = "hotkey_overlay";
        else if (it.key() == HotkeyId::Fullscreen)
            key = "hotkey_fullscreen";
        else if (it.key() == HotkeyId::WindowCapture)
            key = "hotkey_window";
        if (it.value() != _lastSaved[it.key()])
        {
            settings.setValue(key + "/vk", it.value().vk);
            settings.setValue(key + "/modifiers", it.value().modifiers);
            emit hotkeyChanged(it.key(), it.value().modifiers, it.value().vk);
        }
    }

    settings.setValue("fullscreen_preview", _fullscreenPreview->isChecked());
    settings.setValue("window_preview", _windowPreview->isChecked());
    _lastSavedFullscreenPreview = _fullscreenPreview->isChecked();
    _lastSavedWindowPreview = _windowPreview->isChecked();
    _lastSaved = _current;
    _updateStatusLabel();
}

void SettingsWindow::_loadSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    auto load = [&](HotkeyId id, const QString& key, UINT defaultVk, UINT defaultMods)
    {
        HotkeyData data;
        data.vk = settings.value(key + "/vk", defaultVk).toUInt();
        data.modifiers = settings.value(key + "/modifiers", defaultMods).toUInt();
        _lastSaved[id] = data;
        _setCurrent(id, data);
    };

    load(HotkeyId::Overlay, "hotkey_overlay", VK_SNAPSHOT, MOD_NOREPEAT);
    load(HotkeyId::Fullscreen, "hotkey_fullscreen", VK_SNAPSHOT, MOD_SHIFT | MOD_NOREPEAT);
    load(HotkeyId::WindowCapture, "hotkey_window", VK_SNAPSHOT, MOD_ALT | MOD_NOREPEAT);

    _fullscreenPreview->setChecked(settings.value("fullscreen_preview", false).toBool());
    _windowPreview->setChecked(settings.value("window_preview", false).toBool());
    _lastSavedFullscreenPreview = settings.value("fullscreen_preview", false).toBool();
    _lastSavedWindowPreview = settings.value("window_preview", false).toBool();
}

void SettingsWindow::_updateStatusLabel()
{
    bool hotkeysChanged = _current != _lastSaved;
    bool previewChanged = _fullscreenPreview->isChecked() != _lastSavedFullscreenPreview ||
                          _windowPreview->isChecked() != _lastSavedWindowPreview;
    bool hasChanges = hotkeysChanged || previewChanged;
    if (hasChanges)
    {
        _statusLabel->setText("There are unsaved changes!");
        _statusLabel->setStyleSheet("color: rgba(255, 0, 0, 255);");
    }
    else
    {
        _statusLabel->setText("All changes saved.");
        _statusLabel->setStyleSheet("color: rgba(0, 255, 0, 120);");
    }
}

void SettingsWindow::_registerRawInput()
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = (HWND)winId();
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

void SettingsWindow::_unregisterRawInput()
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_REMOVE;
    rid.hwndTarget = NULL;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    if (_isCapturing) _unregisterRawInput();
    QWidget::closeEvent(event);
}

bool SettingsWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    // Immediately cast void pointer as Windows specific messsage.
    // There is no way to avoid using a void pointer.
    // If it ever ISN'T a "MSG*", this will break.
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_INPUT && _isCapturing)
    {
        UINT size = 0;
        GetRawInputData((HRAWINPUT)msg->lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
        QByteArray buffer(size, 0);
        GetRawInputData((HRAWINPUT)msg->lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));

        RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
        if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            const RAWKEYBOARD& kb = raw->data.keyboard;
            if (kb.Flags & RI_KEY_BREAK)
            {
                switch (kb.VKey)
                {
                    case VK_CONTROL:
                    case VK_SHIFT:
                    case VK_MENU:
                    case VK_LWIN:
                    case VK_RWIN: break;
                    default:
                        HotkeyData data;
                        data.vk = kb.VKey;
                        data.modifiers = MOD_NOREPEAT;
                        if (GetKeyState(VK_CONTROL) & 0x8000) data.modifiers |= MOD_CONTROL;
                        if (GetKeyState(VK_SHIFT) & 0x8000) data.modifiers |= MOD_SHIFT;
                        if (GetKeyState(VK_MENU) & 0x8000) data.modifiers |= MOD_ALT;
                        if (GetKeyState(VK_LWIN) & 0x8000) data.modifiers |= MOD_WIN;
                        if (GetKeyState(VK_RWIN) & 0x8000) data.modifiers |= MOD_WIN;
                        _endCapture(false);
                        _setCurrent(_capturingId, data);
                        break;
                }
            }
        }
        return false;
    }
    return QWidget::nativeEvent(eventType, message, result);
}