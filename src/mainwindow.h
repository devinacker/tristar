/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>

#include "level.h"
#include "mapscene.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    // file menu
    void openFile();
    int  closeFile();

    // help menu
    void showAbout();

    // display text on the statusbar
    void status(const QString &msg);
    
    // toolbar updates
    void setOpenFileActions(bool val);
    void setEditActions(bool val);
    void setUndoRedoActions(bool val = true);

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;

    // Information about the currently open file
    QString fileName;
    bool    fileOpen;

    // The level data
    LevelData level;

    // renderin stuff
    MapScene *scene;

    // various funcs
    void setupSignals();
    void setupActions();
    void updateTitle();
    void setLevel(uint);
};

#endif // MAINWINDOW_H
