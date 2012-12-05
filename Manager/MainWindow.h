#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>
#include <vector>
#include "ServiceWidget.h"

namespace Ui {
class MainWindow;
}

class TabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void about();

    void save();
    void saveAll();
    void exit();

    void openConfig();
    void openLog();
    void closeLogTab(int idx);

    void openConfigFile(QFileInfo file);
    void openLogFile(QFileInfo file);

    void dirtyTab(TabWidget* w, bool dirty);

private:
    void setupServiceWidgets();
    
private:
    Ui::MainWindow *ui;
    std::vector<ServiceWidget*> serviceWidgets;
};

#endif // MAINWINDOW_H
