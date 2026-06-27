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
};