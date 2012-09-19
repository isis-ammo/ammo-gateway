#ifndef SERVICEWIDGET_H
#define SERVICEWIDGET_H

#include <QFileInfo>
#include <QWidget>
#include <QVector>
#include "ServiceDesc.h"

namespace Ui {
class ServiceWidget;
}

class ServiceWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ServiceWidget(const ServiceDesc_t& desc, QWidget *parent = 0);
    ~ServiceWidget();

    QVector<QAction*> actions();

signals:
    void openConfigFile(QFileInfo file);
    void openLogFile(QFileInfo file);

public slots:
    void startSvc();
    void stopSvc();
    void restartSvc();
    void configSvc();
    void logSvc();

private:
    Ui::ServiceWidget *ui;
    ServiceDesc_t desc;
    QAction* actionStart;
    QAction* actionStop;
    QAction* actionRestart;
    QAction* actionConfig;
    QAction* actionLog;
};

#endif // SERVICEWIDGET_H
