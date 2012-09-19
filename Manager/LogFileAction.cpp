#include <QIcon>
#include "LogFileAction.h"

LogFileAction::LogFileAction(const QFileInfo& file, QObject *parent) :
    QAction(parent),
    file(file)
{
    setText(file.fileName());
    setIcon(QIcon(":/icons/service_log.png"));

    connect(this, SIGNAL(triggered()),
            this, SLOT(logFileSelected()));
}

void LogFileAction::logFileSelected()
{
    emit openLogFile(this->file);
}
