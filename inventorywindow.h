#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <QWidget>
#include <QListWidget>
#include "gamestate.h"

class InventoryWindow : public QWidget
{
    Q_OBJECT
public:
    explicit InventoryWindow(GameState *state, QWidget *parent = nullptr);

private slots:
    void refreshList();

private:
    GameState *m_state;
    QListWidget *m_listWidget;
};

#endif