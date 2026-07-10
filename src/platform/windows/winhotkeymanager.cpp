// src/platform/windows/winhotkeymanager.cpp
#include "winhotkeymanager.h"

#define WM_REREGISTER_HOTKEY (WM_APP + 1)
#define WM_PAUSE_HOTKEYS (WM_APP + 2)
#define WM_RESUME_HOTKEYS (WM_APP + 3)
#define WM_UNREGISTER_HOTKEY (WM_APP + 4)

static void hotkeyThreadFunc(WinHotkeyManager* manager, DWORD* threadId)
{
    *threadId = GetCurrentThreadId();

    QMap<UINT, UINT> mods;
    QMap<UINT, UINT> vks;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_HOTKEY) { emit manager->hotkeyFired(static_cast<HotkeyId>(msg.wParam)); }
        else if (msg.message == WM_REREGISTER_HOTKEY)
        {
            UINT id = static_cast<UINT>(msg.wParam);
            UINT modifiers = static_cast<UINT>(msg.lParam >> 16);
            UINT vk = static_cast<UINT>(msg.lParam & 0xFFFF);
            UnregisterHotKey(nullptr, id);
            mods[id] = modifiers;
            vks[id] = vk;
            RegisterHotKey(nullptr, id, modifiers, vk);
        }
        else if (msg.message == WM_PAUSE_HOTKEYS)
        {
            for (auto it = mods.begin(); it != mods.end(); ++it) UnregisterHotKey(nullptr, it.key());
        }
        else if (msg.message == WM_RESUME_HOTKEYS)
        {
            for (auto it = mods.begin(); it != mods.end(); ++it)
                RegisterHotKey(nullptr, it.key(), it.value(), vks[it.key()]);
        }
        else if (msg.message == WM_UNREGISTER_HOTKEY)
        {
            UINT id = static_cast<UINT>(msg.wParam);
            UnregisterHotKey(nullptr, id);
            mods.remove(id);
            vks.remove(id);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    for (auto it = mods.begin(); it != mods.end(); ++it) UnregisterHotKey(nullptr, it.key());
}

WinHotkeyManager::WinHotkeyManager(QObject* parent) : HotkeyManager(parent)
{
    _thread = QThread::create([this]() { hotkeyThreadFunc(this, &_threadId); });
    _thread->setObjectName("HotkeyThread");
    _thread->start();

    // wait until thread has set its ID
    while (_threadId == 0) QThread::msleep(1);
}

WinHotkeyManager::~WinHotkeyManager()
{
    PostThreadMessageW(_threadId, WM_QUIT, 0, 0);
    _thread->wait();
    delete _thread;
}

void WinHotkeyManager::registerHotkey(HotkeyId id, quint32 modifiers, quint32 vk)
{
    LPARAM lParam = (static_cast<LPARAM>(modifiers) << 16) | static_cast<LPARAM>(vk);
    _postMessage(WM_REREGISTER_HOTKEY, static_cast<WPARAM>(id), lParam);
}

void WinHotkeyManager::unregisterHotkey(HotkeyId id) { _postMessage(WM_UNREGISTER_HOTKEY, static_cast<WPARAM>(id)); }

void WinHotkeyManager::unregisterAll() { _postMessage(WM_PAUSE_HOTKEYS); }

void WinHotkeyManager::pause() { _postMessage(WM_PAUSE_HOTKEYS); }

void WinHotkeyManager::resume() { _postMessage(WM_RESUME_HOTKEYS); }

void WinHotkeyManager::_postMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (_threadId != 0) PostThreadMessageW(_threadId, msg, wParam, lParam);
}