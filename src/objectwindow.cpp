#include <QTreeWidget>
#include <QMessageBox>

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
    enemyRoot.setText(0, "Enemy Types");
    objectRoot.setText(0, "Objects");
    itemRoot.setText(0, "Items");

    ui->tree->insertTopLevelItem(0, &stateRoot);
    ui->tree->insertTopLevelItem(1, &enemyRoot);
    ui->tree->insertTopLevelItem(2, &objectRoot);
    ui->tree->insertTopLevelItem(3, &itemRoot);

    connect(ui->tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(showInfo(QTreeWidgetItem*,int)));
}

void ObjectWindow::setLevel(const LevelData *level) {
    this->level = level;
}

void ObjectWindow::update() {
    if (!level) return;

    QList<QTreeWidgetItem*> children;

    // update enemy states
    children = stateRoot.takeChildren();
    for (int i = 0; i < children.size(); i++) {
        delete children[i];
    }
    QList<QString> states = level->states.keys();
    stateRoot.setText(0, QString("States (%1)").arg(states.size()));
    for (int i = 0; i < states.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&stateRoot);
        item->setText(0, states[i]);
        item->setData(1, 0, i);
    }

    // update enemies
    children = enemyRoot.takeChildren();
    for (int i = 0; i < children.size(); i++) {
        delete children[i];
    }
    enemyRoot.setText(0, QString("Enemy Types (%1)").arg(level->enemies.size()));
    for (int i = 0; i < level->enemies.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&enemyRoot);
        const enemy_t &enemy = level->enemies[i];
        item->setText(0, QString("%1 (%2)")
                       .arg(enemy.name).arg(enemy.state));
        item->setData(1, 0, i);
    }

    // update objects
    children = objectRoot.takeChildren();
    for (int i = 0; i < children.size(); i++) {
        delete children[i];
    }
    objectRoot.setText(0, QString("Objects (%1)").arg(level->objects.size()));
    for (int i = 0; i < level->objects.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&objectRoot);
        const object_t &object = level->objects[i];
        QString name = "invalid";
        if (object.type < level->objectNames.size())
            name = level->objectNames[object.type];

        item->setText(0, QString("(%1, %2) %3")
                      .arg(object.x).arg(object.y).arg(name));
        item->setData(1, 0, i);
    }

    // update items
    children = itemRoot.takeChildren();
    for (int i = 0; i < children.size(); i++) {
        delete children[i];
    }
    itemRoot.setText(0, QString("Items (%1)").arg(level->items.size()));
    for (int i = 0; i < level->items.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(&itemRoot);
        const item_t &theItem = level->items[i];
        item->setText(0, QString("(%1, %2) Item")
                      .arg(theItem.x).arg(theItem.y));
        item->setData(1, 0, i);
    }
}

void ObjectWindow::showInfo(QTreeWidgetItem *item, int /* column */) {
    QString info;
    QTreeWidgetItem *parent = item->parent();

    int num = item->data(1, 0).toInt();

    // show enemy state info
    if (parent == &stateRoot) {
        QStringList states = level->states.keys();
        const enemystate_t state = level->states[states[num]];
        info = "%1, %2, %3, %4, %5, %6, %7, %8";
        for (uint i = 0; i < 8; i++)
            info = info.arg(state.data[i]);
    } else if (parent == &objectRoot) {
        const object_t obj = level->objects[num];
        info = "%1, %2, %3, %4, %5, %6, %7, %8, %9, %10";
        for (uint i = 0; i < 10; i++)
            info = info.arg(obj.data[i]);
    } else if (parent == &itemRoot) {
        const item_t itm = level->items[num];
        info = "%1, %2, %3, %4";
        for (uint i = 0; i < 3; i++)
            info = info.arg(itm.data[i]);
        info = info.arg(itm.data2);
    }

    if (!info.isNull()) {
        QMessageBox::information(this, item->text(0), info, QMessageBox::Ok);
    }
}

ObjectWindow::~ObjectWindow()
{
    delete ui;
}
