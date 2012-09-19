#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include "LogFileAction.h"
#include "ServiceWidget.h"
#include "ui_ServiceWidget.h"

ServiceWidget::ServiceWidget(const ServiceDesc_t& desc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServiceWidget),
    desc(desc)
{
    ui->setupUi(this);

    ui->serviceBox->setTitle(QString(desc.humanName.c_str()));
    ui->logButton->setMenu(new QMenu());

    if (desc.configName.empty()) {
        ui->configButton->setEnabled(false);
    }
    if (desc.logName.empty()) {
        ui->logButton->setEnabled(false);
    }

    connect(ui->startButton, SIGNAL(clicked()),
            this, SLOT(startSvc()));
    connect(ui->stopButton, SIGNAL(clicked()),
            this, SLOT(stopSvc()));
    connect(ui->restartButton, SIGNAL(clicked()),
            this, SLOT(restartSvc()));
    connect(ui->configButton, SIGNAL(clicked()),
            this, SLOT(configSvc()));
    connect(ui->logButton, SIGNAL(clicked()),
            this, SLOT(logSvc()));
}

ServiceWidget::~ServiceWidget()
{
    delete ui;
}

void ServiceWidget::startSvc()
{
}

void ServiceWidget::stopSvc()
{
}

void ServiceWidget::restartSvc()
{
}

void ServiceWidget::configSvc()
{
    QFileInfo file(QString("C:\\ProgramData\\ammo-gateway\\%1").arg(this->desc.configName.c_str()));

    emit openConfigFile(file);
}

void ServiceWidget::logSvc()
{
    QDir dir("C:\\ProgramData\\ammo-gateway");
    QStringList filter(QString("*%1.log").arg(this->desc.logName.c_str()));
    QFileInfoList files;

    if (!this->desc.logName.empty()) {
        files = dir.entryInfoList(filter, QDir::Files, QDir::Name | QDir::Reversed);
    }

    QMenu* menu = new QMenu();
    foreach (QFileInfo file, files) {
        LogFileAction* action = new LogFileAction(file);
        menu->addAction(action);
        connect(action, SIGNAL(openLogFile(QFileInfo)),
                this, SIGNAL(openLogFile(QFileInfo)));
    }

    ui->logButton->setMenu(menu);
    ui->logButton->showMenu();
}
