#pragma once
#include <QObject>
#include <QWidget>

class KeyCaptureManager : public QObject
{
    Q_OBJECT
   public:
    explicit KeyCaptureManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~KeyCaptureManager() = default;

    virtual void startCapture(WId windowId) = 0;
    virtual void stopCapture() = 0;

   signals:
    void keyCaptured(quint32 vk, quint32 modifiers);
};