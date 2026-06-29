#include <QApplication>
#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
// #include <QDebug>
#include <windows.h>
#include <future>
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
    QObject::connect(
        &app, &QCoreApplication::aboutToQuit,
        [&]()
        {
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