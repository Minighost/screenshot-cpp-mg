#pragma once
#include <QObject>
#include "types.h"

class HotkeyManager : public QObject
{
    Q_OBJECT
   public:
    explicit HotkeyManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~HotkeyManager() = default;

    virtual void registerHotkey(HotkeyId id, quint32 modifiers, quint32 vk) = 0;
    virtual void unregisterHotkey(HotkeyId id) = 0;
    virtual void unregisterAll() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

   signals:
    void hotkeyFired(HotkeyId id);
};