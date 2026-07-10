#pragma once
#include "keycapturemanager.h"

class WinKeyCaptureManager : public KeyCaptureManager
{
    Q_OBJECT
   public:
    explicit WinKeyCaptureManager(QObject* parent = nullptr) : KeyCaptureManager(parent) {}

    void startCapture(WId windowId) override;
    void stopCapture() override;
    bool handleNativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

   private:
    bool _capturing = false;
};