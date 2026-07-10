#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <QStandardPaths>
#include "captureoverlay.h"
#include "windowoverlay.h"
#include "settings.h"
#include "utils.h"
#include "platform.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // initialize settings
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    auto initDefault = [&](const QString& key, const QVariant& value)
    {
        if (!settings.contains(key)) settings.setValue(key, value);
    };
    initDefault("hotkey_overlay/vk", VK_SNAPSHOT);
    initDefault("hotkey_overlay/modifiers", MOD_NOREPEAT);
    initDefault("hotkey_fullscreen/vk", VK_SNAPSHOT);
    initDefault("hotkey_fullscreen/modifiers", MOD_SHIFT | MOD_NOREPEAT);
    initDefault("hotkey_window/vk", VK_SNAPSHOT);
    initDefault("hotkey_window/modifiers", MOD_ALT | MOD_NOREPEAT);
    initDefault("non_persistent", false);
    initDefault("action_region", 0);
    initDefault("action_fullscreen", 0);
    initDefault("action_window", 0);
    initDefault("save_path", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    // tray icon
    QSystemTrayIcon tray;
    tray.setIcon(QIcon(":/screenshot-mg.ico"));
    tray.setToolTip("Screenshot Tool");

    QMenu* menu = new QMenu();
    QAction* settingsAction = new QAction("Settings...", menu);
    QAction* exitAction = new QAction("Exit", menu);
    menu->addAction(settingsAction);
    menu->addAction(exitAction);
    tray.setContextMenu(menu);
    tray.show();

    // platform
    HotkeyManager* hotkeyManager = createHotkeyManager(&app);

    // load saved hotkeys
    hotkeyManager->registerHotkey(
        HotkeyId::Overlay, settings.value("hotkey_overlay/modifiers").toUInt(),
        settings.value("hotkey_overlay/vk").toUInt()
    );
    hotkeyManager->registerHotkey(
        HotkeyId::Fullscreen, settings.value("hotkey_fullscreen/modifiers").toUInt(),
        settings.value("hotkey_fullscreen/vk").toUInt()
    );
    hotkeyManager->registerHotkey(
        HotkeyId::WindowCapture, settings.value("hotkey_window/modifiers").toUInt(),
        settings.value("hotkey_window/vk").toUInt()
    );

    // overlays
    CaptureOverlay captureOverlay;
    WindowOverlay windowOverlay;

    // hotkey signals
    QObject::connect(
        hotkeyManager, &HotkeyManager::hotkeyFired, &app,
        [&](HotkeyId id)
        {
            if (id == HotkeyId::Overlay)
                captureOverlay.show();
            else if (id == HotkeyId::Fullscreen)
                performCaptureAction(
                    grabFullscreenAtCursor(), static_cast<CaptureAction>(settings.value("action_fullscreen").toInt())
                );
            else if (id == HotkeyId::WindowCapture)
                windowOverlay.show();
        },
        Qt::QueuedConnection
    );

    // settings window
    SettingsWindow* settingsWindow = nullptr;
    QObject::connect(
        settingsAction, &QAction::triggered,
        [&]()
        {
            if (!settingsWindow)
            {
                settingsWindow = new SettingsWindow();
                QObject::connect(
                    settingsWindow, &SettingsWindow::hotkeyChanged, [&](HotkeyId id, quint32 modifiers, quint32 vk)
                    { hotkeyManager->registerHotkey(id, modifiers, vk); }
                );
                QObject::connect(
                    settingsWindow, &QObject::destroyed,
                    [&]()
                    {
                        settingsWindow = nullptr;
                        hotkeyManager->resume();
                    }
                );
            }
            hotkeyManager->pause();
            settingsWindow->show();
            settingsWindow->raise();
            settingsWindow->activateWindow();
        }
    );

    QObject::connect(exitAction, &QAction::triggered, &app, &QApplication::quit);

    QObject::connect(
        &app, &QCoreApplication::aboutToQuit,
        [&]()
        {
            if (settingsWindow) settingsWindow->close();
        }
    );

    return app.exec();
}