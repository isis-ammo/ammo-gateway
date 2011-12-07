#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui/QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->tabWidget);
    connect(ui->actionOpen, SIGNAL(activated()), this, SLOT(pickFolder()));
}

void MainWindow::pickFolder() {
    QFileDialog *fileChooser = new QFileDialog(this);
    fileChooser->setAcceptMode(QFileDialog::AcceptOpen);
    fileChooser->setFileMode(QFileDialog::Directory);
    fileChooser->setOption(QFileDialog::ShowDirsOnly);
    int result = fileChooser->exec();
    if(result == QFileDialog::Accepted) {
        if(fileChooser->selectedFiles().length() == 1) {
            ui->statusBar->showMessage("Picked the directory " + fileChooser->selectedFiles()[0]);
        } else {
            ui->statusBar->showMessage("Oops? (No directory or more than one)");
        }
    } else {
        ui->statusBar->showMessage("File dialog cancelled");
    }
    delete fileChooser;
}

MainWindow::~MainWindow()
{
    delete ui;
}
