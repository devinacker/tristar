#include <QTreeWidget>

#include "objectwindow.h"
#include "ui_objectwindow.h"
#include "level.h"

ObjectWindow::ObjectWindow(QWidget *parent, const LevelData *level) :
    QWidget(parent,
            Qt::Window
            | Qt::Dialog
            | Qt::Tool
            | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint),
    ui(new Ui::ObjectWindow),
    level(level)
{
    ui->setupUi(this);

    stateRoot.setText(0, "States");
    enemyRoot.setText(0, "Enemies");
    objectRoot.setText(0, "Objects");
    otherRoot.setText(0, "Unknown");

    ui->tree->insertTopLevelItem(0, &stateRoot);
    ui->tree->insertTopLevelItem(1, &enemyRoot);
    ui->tree->insertTopLevelItem(2, &objectRoot);
    ui->tree->insertTopLevelItem(3, &otherRoot);
}

void ObjectWindow::setLevel(const LevelData *level) {
    this->level = level;
}

void ObjectWindow::update() {
    if (!level) return;

    QList<QTreeWidgetItem*> children;

    // update enemy states
    children = stateRoot.takeChildren();
    for (uint i = 0; i < children.size(); i++) {
        delete children[i];
    }
    QList<QString> states = level->states.keys();
    stateRoot.setText(0, QString("States (%1)").arg(states.size()));
    for (uint i = 0; i < states.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&stateRoot);
        item->setText(0, states[i]);
        item->setData(1, 0, i);
    }

    // update enemies
    children = enemyRoot.takeChildren();
    for (uint i = 0; i < children.size(); i++) {
        delete children[i];
    }
    enemyRoot.setText(0, QString("Enemies (%1)").arg(level->enemies.size()));
    for (uint i = 0; i < level->enemies.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&enemyRoot);
        const enemy_t &enemy = level->enemies[i];
        item->setText(0, QString("%1 (%2)")
                       .arg(enemy.name).arg(enemy.state));
        item->setData(1, 0, i);
    }

    // update objects
    children = objectRoot.takeChildren();
    for (uint i = 0; i < children.size(); i++) {
        delete children[i];
    }
    objectRoot.setText(0, QString("Objects (%1)").arg(level->objects.size()));
    for (uint i = 0; i < level->objects.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&objectRoot);
        const object_t &object = level->objects[i];
        QString name = "invalid";
        if (object.type < level->objectNames.size())
            name = level->objectNames[object.type];

        item->setText(0, QString("(%1, %2) %3")
                      .arg(object.x).arg(object.y).arg(name));
        item->setData(1, 0, i);
    }

    // update unknown thing
    children = otherRoot.takeChildren();
    for (uint i = 0; i < children.size(); i++) {
        delete children[i];
    }
    otherRoot.setText(0, QString("Unknown (%1)").arg(level->data5.size()));
    for (uint i = 0; i < level->data5.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&otherRoot);
        const data5_t &other = level->data5[i];
        item->setText(0, QString("(%1, %2) Unknown %3")
                      .arg(other.x).arg(other.y).arg(other.type));
        item->setData(1, 0, i);
    }
}

ObjectWindow::~ObjectWindow()
{
    delete ui;
}
