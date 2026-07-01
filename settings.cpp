#include "settings.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSettings>
#include <QMessageBox>

SettingsWindow::SettingsWindow(QWidget* parent)
    : QWidget(parent), _hotkeyEdit(new NativeKeyEdit(this)), _saveButton(new QPushButton("Save", this))
{
    setWindowTitle("Settings");
    setAttribute(Qt::WA_DeleteOnClose);

    _hotkeyEdit->setMaximumSequenceLength(1);

    QFormLayout* form = new QFormLayout();
    form->addRow("Screenshot Hotkey:", _hotkeyEdit);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addLayout(form);
    layout->addWidget(_saveButton);
    setLayout(layout);

    connect(_saveButton, &QPushButton::clicked, this, &SettingsWindow::_save);
    connect(_hotkeyEdit, &QKeySequenceEdit::keySequenceChanged, this, &SettingsWindow::_updateSaveButtonState);

    _loadSettings();
    _updateSaveButtonState();
}

void SettingsWindow::_loadSettings()
{
    QSettings settings;
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

    QSettings settings;
    settings.setValue("hotkey", _hotkeyEdit->keySequence().toString());

    _lastSaved = _hotkeyEdit->keySequence();
    _updateSaveButtonState();  // re-disable, since current == lastSaved now
}