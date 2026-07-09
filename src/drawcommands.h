#pragma once
#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>

class AddItemCommand : public QUndoCommand
{
   public:
    AddItemCommand(QGraphicsScene* scene, QGraphicsItem* item) : _scene(scene), _item(item) {}
    ~AddItemCommand()
    {
        if (!_scene->items().contains(_item)) delete _item;
    }

    void undo() override { _scene->removeItem(_item); }

    void redo() override
    {
        if (!_scene->items().contains(_item)) _scene->addItem(_item);
    }

   private:
    QGraphicsScene* _scene;
    QGraphicsItem* _item;
};