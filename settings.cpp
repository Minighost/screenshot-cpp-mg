#include "settings.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSettings>
#include <QMessageBox>
#include "utils.h"

SettingsWindow::SettingsWindow(QWidget* parent)
    : QWidget(parent), _hotkeyEdit(new NativeKeyEdit(this)), _saveButton(new QPushButton("Save", this))
{
    setWindowTitle("Settings");
    setAttribute(Qt::WA_DeleteOnClose);
    _hotkeyEdit->setMaximumSequenceLength(1);

    QPushButton* changeButton = new QPushButton(tintedIcon(":/svgs/draw.svg"), "", this);
    QPushButton* clearButton = new QPushButton(tintedIcon(":/svgs/delete.svg"), "", this);

    QHBoxLayout* hotkeyLayout = new QHBoxLayout();
    hotkeyLayout->addWidget(_hotkeyEdit);
    hotkeyLayout->addWidget(changeButton);
    hotkeyLayout->addWidget(clearButton);

    QFormLayout* form = new QFormLayout();
    form->addRow("Screenshot Hotkey:", hotkeyLayout);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addLayout(form);
    layout->addWidget(_saveButton);
    setLayout(layout);

    connect(
        changeButton, &QPushButton::clicked, this,
        [this, changeButton]()
        {
            if (_hotkeyEdit->isCapturing())
            {
                _hotkeyEdit->setKeySequence(_lastSaved);  // revert display
                _hotkeyEdit->endCapture();
                changeButton->setIcon(tintedIcon(":/svgs/draw.svg"));
            }
            else
            {
                _hotkeyEdit->beginCapture();
                changeButton->setIcon(tintedIcon(":/svgs/cancel.svg"));
            }
        }
    );
    connect(
        clearButton, &QPushButton::clicked, this,
        [this, changeButton]()
        {
            _hotkeyEdit->clear();
            _hotkeyEdit->endCapture();
            changeButton->setIcon(tintedIcon(":/svgs/draw.svg"));
            _updateSaveButtonState();
        }
    );
    connect(_saveButton, &QPushButton::clicked, this, &SettingsWindow::_save);
    connect(
        _hotkeyEdit, &QKeySequenceEdit::keySequenceChanged, this,
        [this, changeButton]()
        {
            if (!_hotkeyEdit->isCapturing()) changeButton->setIcon(tintedIcon(":/svgs/draw.svg"));
            _updateSaveButtonState();
        }
    );

    _loadSettings();
    _updateSaveButtonState();
}

void SettingsWindow::_loadSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    QString saved = settings.value("hotkey", "Print Screen").toString();
    _lastSaved = QKeySequence::fromString(saved);
    _hotkeyEdit->setKeySequence(_lastSaved);
}

void SettingsWindow::_updateSaveButtonState()
{
    bool changed = _hotkeyEdit->hasNewKey() && (_hotkeyEdit->keySequence() != _lastSaved);
    _saveButton->setEnabled(changed);
}

void SettingsWindow::_save()
{
    if (!_hotkeyEdit->hasNewKey()) return;

    emit hotkeyChanged(_hotkeyEdit->nativeModifiers(), _hotkeyEdit->nativeVk());

    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    settings.setValue("hotkey", _hotkeyEdit->keySequence().toString());

    _lastSaved = _hotkeyEdit->keySequence();
    _updateSaveButtonState();  // re-disable, since current == lastSaved now
}