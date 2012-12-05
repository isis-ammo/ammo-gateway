#ifndef LOGFILEACTION_H
#define LOGFILEACTION_H

#include <QAction>
#include <QFileInfo>

class LogFileAction : public QAction
{
    Q_OBJECT
public:
    explicit LogFileAction(const QFileInfo& file, QObject *parent = 0);
    
signals:
    void openLogFile(QFileInfo file);
    
private slots:
    void logFileSelected();
    
private:
    QFileInfo file;
};

#endif // LOGFILEACTION_H
