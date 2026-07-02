#include "settings.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include "utils.h"

SettingsWindow::SettingsWindow(QWidget* parent)
    : QWidget(parent),
      _hotkeyLabel(new QLabel(this)),
      _saveButton(new QPushButton("Save", this)),
      _changeButton(new QPushButton(tintedIcon(":/svgs/draw.svg", 16), "", this)),
      _clearButton(new QPushButton(tintedIcon(":/svgs/delete.svg", 16), "", this))
{
    setWindowTitle("Settings");
    setAttribute(Qt::WA_DeleteOnClose);

    _changeButton->setFixedSize(24, 24);
    _clearButton->setFixedSize(24, 24);

    _changeButton->setIconSize(QSize(12, 12));
    _clearButton->setIconSize(QSize(12, 12));

    QHBoxLayout* screenshotRegionLayout = new QHBoxLayout();
    screenshotRegionLayout->setSpacing(1);
    screenshotRegionLayout->addWidget(new QLabel("Screenshot: ", this));
    screenshotRegionLayout->addWidget(_hotkeyLabel);
    screenshotRegionLayout->addStretch();
    screenshotRegionLayout->addWidget(_changeButton);
    screenshotRegionLayout->addWidget(_clearButton);

    QLabel* keybind_notice = new QLabel("Keybinds are disabled while this window is open!", this);

    // actual layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(keybind_notice);
    layout->addLayout(screenshotRegionLayout);
    layout->addWidget(_saveButton);
    setLayout(layout);

    connect(
        _changeButton, &QPushButton::clicked, this,
        [this]()
        {
            if (_isCapturing)
                _endCapture(true);  // revert
            else
                _beginCapture();
        }
    );

    connect(
        _clearButton, &QPushButton::clicked, this,
        [this]()
        {
            _endCapture(false);
            _setCurrent({});
        }
    );

    connect(_saveButton, &QPushButton::clicked, this, &SettingsWindow::_save);

    _loadSettings();
    _updateSaveButtonState();
}

void SettingsWindow::_beginCapture()
{
    _isCapturing = true;
    _changeButton->setIcon(tintedIcon(":/svgs/cancel.svg", 16));
    _hotkeyLabel->setText("Press a key...");
    _registerRawInput();
}

void SettingsWindow::_endCapture(bool revert)
{
    _isCapturing = false;
    _changeButton->setIcon(tintedIcon(":/svgs/draw.svg", 16));
    _unregisterRawInput();
    if (revert) _setCurrent(_current);  // redisplay current without clearing it
}

void SettingsWindow::_setCurrent(const HotkeyData& data)
{
    _current = data;
    if (data.isEmpty())
        _hotkeyLabel->setText("(none)");
    else
        _hotkeyLabel->setText(hotkeyToDisplayString(data.vk, data.modifiers));
    _updateSaveButtonState();
}

void SettingsWindow::_save()
{
    if (_current == _lastSaved) return;

    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    settings.setValue("hotkey/vk", _current.vk);
    settings.setValue("hotkey/modifiers", _current.modifiers);

    emit hotkeyChanged(_current.modifiers, _current.vk);

    _lastSaved = _current;
    _updateSaveButtonState();
}

void SettingsWindow::_loadSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    HotkeyData data;
    data.vk = settings.value("hotkey/vk", VK_SNAPSHOT).toUInt();
    data.modifiers = settings.value("hotkey/modifiers", MOD_NOREPEAT).toUInt();
    _lastSaved = data;
    _setCurrent(data);
}

void SettingsWindow::_updateSaveButtonState() { _saveButton->setEnabled(!(_current == _lastSaved)); }

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
                qDebug() << "Raw Input VKey:" << kb.VKey << "Flags:" << kb.Flags << "MakeCode:" << kb.MakeCode;
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
                        _setCurrent(data);
                        break;
                }
            }
        }
        return false;
    }
    return QWidget::nativeEvent(eventType, message, result);
}