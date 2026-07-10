#pragma once
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QWidget>

class WindowPicker : public QObject
{
    Q_OBJECT
   public:
    explicit WindowPicker(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~WindowPicker() = default;

    virtual QRect windowAtPoint(QPoint screenPos, WId excludeWindow) = 0;
};