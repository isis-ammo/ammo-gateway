#ifndef LOGFILEWIDGET_H
#define LOGFILEWIDGET_H

#include <QFile>
#include <QFileSystemWatcher>
#include "TabWidget.h"

namespace Ui {
class LogFileWidget;
}

class LogFileWidget : public TabWidget
{
    Q_OBJECT
    
public:
    explicit LogFileWidget(const QString& logFileName, QWidget *parent = 0);
    ~LogFileWidget();

private slots:
    void moreDataAvailable(const QString&);
    
private:
    Ui::LogFileWidget *ui;
    QFile* file;
    QFileSystemWatcher watcher;
};

#endif // LOGFILEWIDGET_H
