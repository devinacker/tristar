#ifndef OBJECTWINDOW_H
#define OBJECTWINDOW_H

#include <QWidget>
#include <QTreeWidget>

#include "level.h"

namespace Ui {
class ObjectWindow;
}

class ObjectWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectWindow(QWidget* parent = 0, const LevelData* = 0);
    ~ObjectWindow();

    void setLevel(const LevelData*);
    void update();

private slots:
    void showInfo(QTreeWidgetItem * item, int column);

private:
    Ui::ObjectWindow *ui;
    const LevelData *level;

    QTreeWidgetItem enemyRoot, typeRoot, objectRoot, itemRoot;
};

#endif // OBJECTWINDOW_H
