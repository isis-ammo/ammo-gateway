#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>
#include <vector>
#include "ServiceWidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void openConfig();
    void openLog();
    void closeLogTab(int idx);

    void openConfigFile(QFileInfo file);
    void openLogFile(QFileInfo file);

private:
    void setupServiceWidgets();
    
private:
    Ui::MainWindow *ui;
    std::vector<ServiceWidget*> serviceWidgets;
};

#endif // MAINWINDOW_H
