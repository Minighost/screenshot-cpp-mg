#pragma once
#include <QObject>

class Communicator : public QObject
{
    Q_OBJECT
   public:
    explicit Communicator(QObject* parent = nullptr) : QObject(parent) {}

   signals:
    void showOverlay();
    void quitApp();
    void hotkeyChanged(quint32 modifiers, quint32 vk);
    void pauseHotkey();
    void resumeHotkey();
};