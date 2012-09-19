#ifndef LOGFILEWIDGET_H
#define LOGFILEWIDGET_H

#include <QWidget>
#include <QFile>
#include <QFileSystemWatcher>

namespace Ui {
class LogFileWidget;
}

class LogFileWidget : public QWidget
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
