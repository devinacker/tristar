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

private:
    Ui::ObjectWindow *ui;
    const LevelData *level;

    QTreeWidgetItem stateRoot, enemyRoot, objectRoot, otherRoot;
};

#endif // OBJECTWINDOW_H
