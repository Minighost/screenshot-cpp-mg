#pragma once
#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsPathItem>

class StrokeCommand : public QUndoCommand
{
   public:
    StrokeCommand(QGraphicsScene* scene, QGraphicsPathItem* item) : _scene(scene), _item(item) {}
    ~StrokeCommand()
    {
        if (!_scene->items().contains(_item)) delete _item;
    }

    void undo() override { _scene->removeItem(_item); }

    void redo() override
    {
        if (_item->scene() != _scene) _scene->addItem(_item);
    }

   private:
    QGraphicsScene* _scene;
    QGraphicsPathItem* _item;
};