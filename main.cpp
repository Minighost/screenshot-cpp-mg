#define WM_REREGISTER_HOTKEY (WM_APP + 1)
#define WM_PAUSE_HOTKEY (WM_APP + 2)
#define WM_RESUME_HOTKEY (WM_APP + 3)
#define WM_PAUSE_ALL_HOTKEYS (WM_APP + 4)
#define WM_RESUME_ALL_HOTKEYS (WM_APP + 5)

#include <QApplication>
#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <QStandardPaths>
// #include <QDebug>
#include <windows.h>
#include <future>
#include "utils.h"
#include "communicator.h"
#include "captureoverlay.h"
#include "settings.h"
#include "windowoverlay.h"
#include "preview.h"
#include "types.h"

void hotkeyThread(Communicator* comm)
{
    HWND hwnd = nullptr;

    // map hotkeyids to some combination of hotkeys
    // reminder: win32 hotkey id 0 is invalid
    // not really sure if this is better or worse than simply
    // subtracting 1 from the hotkeyid
    QMap<UINT, UINT> mods = {{1, MOD_NOREPEAT}, {2, MOD_SHIFT | MOD_NOREPEAT}, {3, MOD_ALT | MOD_NOREPEAT}};
    QMap<UINT, UINT> vks = {{1, VK_SNAPSHOT}, {2, VK_SNAPSHOT}, {3, VK_SNAPSHOT}};

    for (auto it = mods.begin(); it != mods.end(); ++it) RegisterHotKey(hwnd, it.key(), it.value(), vks[it.key()]);

    MSG msg;
    while (GetMessage(&msg, hwnd, 0, 0))
    {
        if (msg.message == WM_HOTKEY)
        {
            if (msg.wParam == 1)
                emit comm->showOverlay();
            else if (msg.wParam == 2)
                emit comm->captureFullscreen();
            else if (msg.wParam == 3)
                emit comm->captureWindow();
        }
        else if (msg.message == WM_REREGISTER_HOTKEY)
        {
            UINT id = static_cast<UINT>(msg.wParam);
            UINT modifiers = static_cast<UINT>(msg.lParam >> 16);
            UINT vk = static_cast<UINT>(msg.lParam & 0xFFFF);
            UnregisterHotKey(hwnd, id);
            mods[id] = modifiers;
            vks[id] = vk;
            RegisterHotKey(hwnd, id, modifiers, vk);
        }
        else if (msg.message == WM_PAUSE_HOTKEY)
        {
            for (auto it = mods.begin(); it != mods.end(); ++it) UnregisterHotKey(hwnd, it.key());
        }
        else if (msg.message == WM_RESUME_HOTKEY)
        {
            for (auto it = mods.begin(); it != mods.end(); ++it)
                RegisterHotKey(hwnd, it.key(), it.value(), vks[it.key()]);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    for (auto it = mods.begin(); it != mods.end(); ++it) UnregisterHotKey(hwnd, it.key());
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // Init Setting file
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


    // Tray Icon
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

    Communicator comm;
    CaptureOverlay captureOverlay;
    WindowOverlay windowOverlay;

    QObject::connect(&comm, &Communicator::showOverlay, &captureOverlay, &CaptureOverlay::show);
    QObject::connect(&comm, &Communicator::captureWindow, &windowOverlay, &WindowOverlay::show);
    QObject::connect(&comm, &Communicator::quitApp, &app, &QApplication::quit);

    std::promise<DWORD> threadIdPromise;
    std::future<DWORD> threadIdFuture = threadIdPromise.get_future();

    QThread* thread = QThread::create(
        [&comm, &threadIdPromise]()
        {
            threadIdPromise.set_value(GetCurrentThreadId());
            hotkeyThread(&comm);
        }
    );
    thread->setObjectName("HotkeyThread");
    thread->start();

    DWORD hotkeyThreadId = threadIdFuture.get();  // blocks until thread sets it

    QObject::connect(exitAction, &QAction::triggered, &app, &QApplication::quit);

    SettingsWindow* settingsWindow = nullptr;
    QObject::connect(
        settingsAction, &QAction::triggered,
        [&]()
        {
            if (!settingsWindow)
            {
                settingsWindow = new SettingsWindow();
                QObject::connect(settingsWindow, &SettingsWindow::hotkeyChanged, &comm, &Communicator::hotkeyChanged);
                QObject::connect(
                    settingsWindow, &QObject::destroyed,
                    [&]()
                    {
                        settingsWindow = nullptr;
                        emit comm.resumeHotkey();
                    }
                );
            }
            emit comm.pauseHotkey();
            settingsWindow->show();
            settingsWindow->raise();
            settingsWindow->activateWindow();
        }
    );

    // Depends on hotkeyThreadId
    QObject::connect(
        &comm, &Communicator::hotkeyChanged,
        [hotkeyThreadId](quint32 id, quint32 modifiers, quint32 vk)
        {
            // need 3 params, but only lparam and wparam are available.
            // this will pack both modifiers and vk into lparam, leaving wparam free for the target id.
            // apparently this is standard practice? idk lol
            LPARAM lParam = (static_cast<LPARAM>(modifiers) << 16) | static_cast<LPARAM>(vk);
            PostThreadMessageW(hotkeyThreadId, WM_REREGISTER_HOTKEY, static_cast<WPARAM>(id), lParam);
        }
    );

    QObject::connect(
        &comm, &Communicator::pauseHotkey,
        [hotkeyThreadId]() { PostThreadMessageW(hotkeyThreadId, WM_PAUSE_HOTKEY, 0, 0); }
    );

    QObject::connect(
        &comm, &Communicator::resumeHotkey,
        [hotkeyThreadId]() { PostThreadMessageW(hotkeyThreadId, WM_RESUME_HOTKEY, 0, 0); }
    );

    QObject::connect(
        &comm, &Communicator::captureFullscreen, &app,
        [&]()
        {
            QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
            CaptureAction action = static_cast<CaptureAction>(settings.value("action_fullscreen", 0).toInt());
            performCaptureAction(grabFullscreenAtCursor(), action);
        },
        Qt::QueuedConnection
    );

    QObject::connect(
        &app, &QCoreApplication::aboutToQuit,
        [&]()
        {
            if (settingsWindow) settingsWindow->close();
            // qDebug() << "hotkeyThreadId:" << hotkeyThreadId;
            BOOL result = PostThreadMessageW(hotkeyThreadId, WM_QUIT, 0, 0);
            // qDebug() << "PostThreadMessageW result:" << result;
            // qDebug() << "GetLastError:" << GetLastError();
            thread->wait();
            delete thread;
        }
    );

    return app.exec();
}