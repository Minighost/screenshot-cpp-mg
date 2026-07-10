#pragma once
#include "hotkeymanager.h"
#include <QThread>
#include <QMap>
#include <windows.h>

class WinHotkeyManager : public HotkeyManager
{
    Q_OBJECT
   public:
    explicit WinHotkeyManager(QObject* parent = nullptr);
    ~WinHotkeyManager() override;

    void registerHotkey(HotkeyId id, quint32 modifiers, quint32 vk) override;
    void unregisterHotkey(HotkeyId id) override;
    void unregisterAll() override;
    void pause() override;
    void resume() override;

   private:
    QThread* _thread;
    DWORD _threadId = 0;

    void _postMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);
};