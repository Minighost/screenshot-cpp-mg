#include "settings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QLineEdit>
#include <QFileDialog>
#include <QStandardPaths>
// #include <QDebug>
#include "utils.h"
#include "platform.h"

const QMap<HotkeyId, HotkeyData> SettingsWindow::DEFAULT_HOTKEYS = {
    {HotkeyId::Overlay, {VK_SNAPSHOT, MOD_NOREPEAT}},
    {HotkeyId::Fullscreen, {VK_SNAPSHOT, MOD_SHIFT | MOD_NOREPEAT}},
    {HotkeyId::WindowCapture, {VK_SNAPSHOT, MOD_ALT | MOD_NOREPEAT}},
};

SettingsWindow::SettingsWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("Settings");
    setAttribute(Qt::WA_DeleteOnClose);

    _keyCaptureManager = createKeyCaptureManager(this);
    connect(
        _keyCaptureManager, &KeyCaptureManager::keyCaptured, this,
        [this](quint32 vk, quint32 modifiers)
        {
            _endCapture(false);
            _setCurrent(_capturingId, {vk, modifiers});
        }
    );

    // helpers for layout creation
    auto makeDivider = [this]()
    {
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet("color: rgba(255, 255, 255, 40);");
        return line;
    };

    auto makeSectionTitle = [this](const QString& title)
    {
        QLabel* label = new QLabel(title, this);
        label->setStyleSheet("font-weight: bold;");
        return label;
    };

    // actual layout
    QVBoxLayout* layout = new QVBoxLayout();

    QLabel* keybind_notice = new QLabel("Keybinds are disabled while this window is open!", this);
    keybind_notice->setAlignment(Qt::AlignCenter);
    layout->addWidget(keybind_notice);

    // Hotkeys (these comments are for visually locating sections)
    // (yes i keep getting lost)
    layout->addWidget(makeDivider());
    layout->addWidget(makeSectionTitle("Hotkeys"));

    for (HotkeyId id : {HotkeyId::Overlay, HotkeyId::Fullscreen, HotkeyId::WindowCapture})
    {
        HotkeyRow row = _makeHotkeyRow(id);
        _rows[id] = row;

        QString rowLabel;
        if (id == HotkeyId::Overlay)
            rowLabel = "Region:";
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

    // Actions
    layout->addWidget(makeDivider());
    layout->addWidget(makeSectionTitle("Actions"));

    _nonPersistent = new QCheckBox("Instant region selection (enables selected action)", this);

    auto makeActionRow = [this](const QString& label)
    {
        QComboBox* combo = new QComboBox(this);
        combo->addItem("Copy");
        combo->addItem("Save");
        combo->addItem("Preview");

        QHBoxLayout* row = new QHBoxLayout();
        QLabel* lbl = new QLabel(label, this);
        lbl->setFixedWidth(80);
        row->addWidget(lbl);
        row->addWidget(combo);
        row->addStretch();
        return QPair<QHBoxLayout*, QComboBox*>(row, combo);
    };

    auto [regionRow, regionCombo] = makeActionRow("Instant Region:");
    auto [fullscreenRow, fullscreenCombo] = makeActionRow("Fullscreen:");
    auto [windowRow, windowCombo] = makeActionRow("Window:");

    _regionAction = regionCombo;
    _fullscreenAction = fullscreenCombo;
    _windowAction = windowCombo;

    layout->addWidget(_nonPersistent);
    layout->addLayout(regionRow);
    layout->addLayout(fullscreenRow);
    layout->addLayout(windowRow);

    connect(_nonPersistent, &QCheckBox::checkStateChanged, this, &SettingsWindow::_updateStatusLabel);
    connect(_regionAction, &QComboBox::currentIndexChanged, this, &SettingsWindow::_updateStatusLabel);
    connect(_fullscreenAction, &QComboBox::currentIndexChanged, this, &SettingsWindow::_updateStatusLabel);
    connect(_windowAction, &QComboBox::currentIndexChanged, this, &SettingsWindow::_updateStatusLabel);

    // Save path
    layout->addWidget(makeDivider());
    layout->addWidget(makeSectionTitle("Save"));

    QLabel* saveNotice = new QLabel("Only used for auto-save.", this);
    saveNotice->setStyleSheet("color: rgba(255,255,255,150); font-size: 11px;");

    _savePath = new QLineEdit(this);
    _savePath->setReadOnly(true);

    QPushButton* browseButton = new QPushButton("Browse...", this);

    QHBoxLayout* pathRow = new QHBoxLayout();
    pathRow->addWidget(_savePath);
    pathRow->addWidget(browseButton);

    layout->addWidget(saveNotice);
    layout->addLayout(pathRow);

    connect(
        browseButton, &QPushButton::clicked, this,
        [this]()
        {
            QString path = QFileDialog::getExistingDirectory(this, "Select Save Folder", _savePath->text());
            if (!path.isEmpty())
            {
                _savePath->setText(path);
                _updateStatusLabel();
            }
        }
    );

    connect(_savePath, &QLineEdit::textChanged, this, &SettingsWindow::_updateStatusLabel);

    // Status + save/cancel buttons
    layout->addWidget(makeDivider());

    layout->addSpacing(5);
    _statusLabel = new QLabel("All settings saved.", this);
    _statusLabel->setStyleSheet("color: rgba(255, 255, 255, 120);");
    layout->addWidget(_statusLabel);
    layout->addSpacing(5);

    QHBoxLayout* saveCancelRow = new QHBoxLayout();
    _saveButton = new QPushButton("Save", this);
    _cancelButton = new QPushButton("Cancel", this);

    connect(_saveButton, &QPushButton::clicked, this, &SettingsWindow::_save);
    connect(_cancelButton, &QPushButton::clicked, this, &SettingsWindow::close);

    saveCancelRow->addWidget(_saveButton);
    saveCancelRow->addWidget(_cancelButton);

    layout->addLayout(saveCancelRow);

    setLayout(layout);
    _loadSettings();
}

HotkeyRow SettingsWindow::_makeHotkeyRow(HotkeyId id)
{
    HotkeyRow row;
    row.label = new QLabel(this);
    row.changeButton = new QPushButton(tintedIcon(":/draw.svg", 16), "", this);
    row.clearButton = new QPushButton(tintedIcon(":/delete.svg", 16), "", this);
    row.revertButton = new QPushButton(tintedIcon(":/reset-zoom-level.svg", 16), "", this);

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
    if (_isCapturing) _endCapture(true);
    _isCapturing = true;
    _capturingId = id;
    _rows[id].changeButton->setIcon(tintedIcon(":/cancel.svg", 16));
    _rows[id].label->setText("Press a key...");
    _keyCaptureManager->startCapture(winId());
}

void SettingsWindow::_endCapture(bool revert)
{
    if (!_isCapturing) return;
    _rows[_capturingId].changeButton->setIcon(tintedIcon(":/draw.svg", 16));
    _keyCaptureManager->stopCapture();
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
        if (it.value().vk != settings.value(key + "/vk").toUInt() ||
            it.value().modifiers != settings.value(key + "/modifiers").toUInt())
        {
            settings.setValue(key + "/vk", it.value().vk);
            settings.setValue(key + "/modifiers", it.value().modifiers);
            emit hotkeyChanged(it.key(), it.value().modifiers, it.value().vk);
        }
    }

    settings.setValue("non_persistent", _nonPersistent->isChecked());
    settings.setValue("action_region", _regionAction->currentIndex());
    settings.setValue("action_fullscreen", _fullscreenAction->currentIndex());
    settings.setValue("action_window", _windowAction->currentIndex());
    settings.setValue("save_path", _savePath->text());

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
        _setCurrent(id, data);
    };

    load(HotkeyId::Overlay, "hotkey_overlay", VK_SNAPSHOT, MOD_NOREPEAT);
    load(HotkeyId::Fullscreen, "hotkey_fullscreen", VK_SNAPSHOT, MOD_SHIFT | MOD_NOREPEAT);
    load(HotkeyId::WindowCapture, "hotkey_window", VK_SNAPSHOT, MOD_ALT | MOD_NOREPEAT);

    _nonPersistent->setChecked(settings.value("non_persistent", false).toBool());
    _regionAction->setCurrentIndex(settings.value("action_region", 0).toInt());
    _fullscreenAction->setCurrentIndex(settings.value("action_fullscreen", 0).toInt());
    _windowAction->setCurrentIndex(settings.value("action_window", 0).toInt());

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    _savePath->setText(settings.value("save_path", defaultPath).toString());
}

void SettingsWindow::_updateStatusLabel()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    bool hotkeysChanged = false;
    for (auto it = _current.begin(); it != _current.end(); ++it)
    {
        QString key;
        if (it.key() == HotkeyId::Overlay)
            key = "hotkey_overlay";
        else if (it.key() == HotkeyId::Fullscreen)
            key = "hotkey_fullscreen";
        else if (it.key() == HotkeyId::WindowCapture)
            key = "hotkey_window";
        if (it.value().vk != settings.value(key + "/vk").toUInt() ||
            it.value().modifiers != settings.value(key + "/modifiers").toUInt())
        {
            hotkeysChanged = true;
            break;
        }
    }

    bool behaviorChanged = _nonPersistent->isChecked() != settings.value("non_persistent").toBool() ||
                           _regionAction->currentIndex() != settings.value("action_region").toInt() ||
                           _fullscreenAction->currentIndex() != settings.value("action_fullscreen").toInt() ||
                           _windowAction->currentIndex() != settings.value("action_window").toInt();

    bool pathChanged = _savePath->text() != settings.value("save_path").toString();

    bool hasChanges = hotkeysChanged || behaviorChanged || pathChanged;
    if (hasChanges)
    {
        _statusLabel->setText("There are unsaved changes!");
        _statusLabel->setStyleSheet("color: rgba(255, 0, 0, 200);");
    }
    else
    {
        _statusLabel->setText("All changes saved.");
        _statusLabel->setStyleSheet("color: rgba(255, 255, 255, 120);");
    }
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    if (_isCapturing) _endCapture(true);
    QWidget::closeEvent(event);
}

bool SettingsWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    if (_keyCaptureManager->handleNativeEvent(eventType, message, result)) return true;
    return QWidget::nativeEvent(eventType, message, result);
}