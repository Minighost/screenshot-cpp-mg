#include <QApplication>
#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
#include <windows.h>
#include "communicator.h"
#include "overlay.h"

void hotkeyThread(Communicator* comm)
{
    HWND hwnd = nullptr;
    RegisterHotKey(hwnd, 1, MOD_NOREPEAT, VK_SNAPSHOT);
    MSG msg;
    while (GetMessage(&msg, hwnd, 0, 0))
    {
        if (msg.message == WM_HOTKEY) emit comm->showOverlay();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterHotKey(hwnd, 1);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QSystemTrayIcon tray;
    tray.setIcon(QIcon(":/screenshot-mg.ico"));
    tray.setToolTip("Screenshot Tool");

    QMenu* menu = new QMenu();
    QAction* exitAction = new QAction("Exit", menu);
    QObject::connect(exitAction, &QAction::triggered, &app, &QApplication::quit);
    menu->addAction(exitAction);

    tray.setContextMenu(menu);
    tray.show();

    Communicator comm;
    CaptureOverlay overlay;

    QObject::connect(&comm, &Communicator::showOverlay, &overlay, &CaptureOverlay::show);
    QObject::connect(&comm, &Communicator::quitApp, &app, &QApplication::quit);

    QThread* thread = QThread::create([&comm]() { hotkeyThread(&comm); });
    thread->setObjectName("HotkeyThread");
    thread->start();

    return app.exec();
}