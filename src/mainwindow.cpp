/*
  mainwindow.cpp

  This code is released under the terms of the MIT license.
  See COPYING.txt for details.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

#include <cstdio>
#include <cstdlib>

#include "level.h"
#include "mapscene.h"
#include "objectwindow.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fileOpen(false),
    scene(new MapScene(this, &level))
{
    ui->setupUi(this);

    ui->graphicsView->setScene(scene);
    // enable mouse tracking for graphics view
    ui->graphicsView->setMouseTracking(true);
    ui->graphicsView->setBackgroundRole(QPalette::Mid);

    // remove margins around map view and other stuff
    this->centralWidget()->layout()->setContentsMargins(0,0,0,0);

    objWin.setLevel(&level);

    setupSignals();
    setupActions();
    setOpenFileActions(false);
    updateTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete scene;
}

void MainWindow::setupSignals() {
    // file menu
    QObject::connect(ui->action_Open, SIGNAL(triggered()),
                     this, SLOT(openFile()));
    QObject::connect(ui->action_Close, SIGNAL(triggered()),
                     this, SLOT(closeFile()));

    QObject::connect(ui->action_Exit, SIGNAL(triggered()),
                     this, SLOT(close()));

    // help menu
    QObject::connect(ui->action_About, SIGNAL(triggered()),
                     this, SLOT(showAbout()));

    // other window-related stuff
    // receive status bar messages from scene
    QObject::connect(scene, SIGNAL(statusMessage(QString)),
                     ui->statusBar, SLOT(showMessage(QString)));
}

void MainWindow::setupActions() {
    // from file menu
    ui->toolBar->addAction(ui->action_Open);
}

/*
  friendly status bar function
*/
void MainWindow::status(const QString& msg) {
    ui->statusBar->showMessage(msg);
}

/*
  actions that are disabled when a file is not open
*/
void MainWindow::setOpenFileActions(bool val) {
    setEditActions(val);
    // setEditActions may disable this
    ui->action_Open->setEnabled(true);
}

/*
  actions that are disabled while saving the file
*/
void MainWindow::setEditActions(bool val) {
    setUndoRedoActions(val);
    ui->action_Close       ->setEnabled(val);
    ui->action_Open        ->setEnabled(val);
}

/*
 *actions that depend on the state of the undo stack
 */
void MainWindow::setUndoRedoActions(bool val) {
    //ui->action_Undo->setEnabled(val && scene->canUndo());
    //ui->action_Redo->setEnabled(val && scene->canRedo());
}

/*
  put the current file name in the title bar
*/
void MainWindow::updateTitle() {
    if (fileOpen)
        this->setWindowTitle(QString("%1 - %2").arg(INFO_TITLE)
                             .arg(QString(fileName)));
    else
        this->setWindowTitle(INFO_TITLE);
}

/*
  main window close
*/
void MainWindow::closeEvent(QCloseEvent *event) {
    if (closeFile() != -1)
        event->accept();
    else
        event->ignore();
}

/*
  File menu item slots
*/
void MainWindow::openFile() {

    // open file dialog
    QString newFileName = QFileDialog::getOpenFileName(this,
                                 tr("Open Map"),
                                 fileName,
                                 tr("Map data (*.dat);;All files (*.*)"));

    if (!newFileName.isNull() && !closeFile()) {
        status(tr("Opening file %1").arg(newFileName));

        // open file
        QFile file(newFileName);
        if (file.open(QFile::ReadOnly)) {
            // check magic
            file.seek(0);
            if (file.read(6) != "XBIN\x34\x12") {
                QMessageBox::information(this,
                                         "Open Map",
                                         "File is not a valid map.",
                                         QMessageBox::Ok);

                file.close();
                return;
            }

            fileName = newFileName;
            fileOpen = true;
            setOpenFileActions(true);

            level.open(file);
            objWin.update();
            objWin.show();

            updateTitle();
            file.close();
        } else {
            QMessageBox::information(this,
                                     "Open Map",
                                     "Unable to open file.",
                                     QMessageBox::Ok);
        }
    }
}

/*
  Close the currently open file, prompting the user to save changes
  if necessary.
  Return values:
    -1: user cancels file close (file remains open)
     0: file closed successfully (or was already closed)
*/
int MainWindow::closeFile() {
    if (!fileOpen)
        return 0;

    scene->cancelSelection();
    scene->refresh();
    scene->clearStack();

    setOpenFileActions(false);
    fileOpen = false;
    updateTitle();

    objWin.hide();

    return 0;
}

/*
  Help menu item slots
*/
void MainWindow::showAbout() {
    QMessageBox::information(this,
                             tr("About"),
                             tr("%1 version %2\nby Devin Acker (Revenant)")
                             .arg(INFO_TITLE).arg(INFO_VERS),
                             QMessageBox::Ok,
                             QMessageBox::Ok);
}
