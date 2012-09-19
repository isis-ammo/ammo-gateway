#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include "ConfigFileWidget.h"
#include "ManagerConfigurationManager.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "LogFileWidget.h"
#include "ServiceDesc.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupServiceWidgets();


    connect(ui->action_Open_Config_File, SIGNAL(triggered()),
            this, SLOT(openConfig()));
    connect(ui->action_Open_Log_File, SIGNAL(triggered()),
            this, SLOT(openLog()));
    connect(ui->actionExit, SIGNAL(triggered()),
            this, SLOT(close()));

    connect(ui->logTabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeLogTab(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openConfig()
{
    QFileDialog dialog(this,
                       tr("Select config file"),
                       "C:\\ProgramData\\ammo-gateway",
                       tr("Config Files (*.json)"));

    if (!dialog.exec()) {
        return;
    }

    foreach (QString filename, dialog.selectedFiles()) {
        QFileInfo fileinfo(filename);
        openConfigFile(fileinfo);
    }
}

void MainWindow::openLog()
{
    QFileDialog dialog(this,
                       tr("Select log file"),
                       "C:\\ProgramData\\ammo-gateway",
                       tr("Log Files (*.log)"));

    if (!dialog.exec()) {
        return;
    }

    foreach (QString filename, dialog.selectedFiles()) {
        QFileInfo fileinfo(filename);
        openLogFile(fileinfo);
    }
}

void MainWindow::closeLogTab(int idx)
{
    QWidget* logWidget = ui->logTabWidget->widget(idx);

    if (logWidget) {
        ui->logTabWidget->removeTab(idx);
        delete logWidget;
    }
}

void MainWindow::openConfigFile(QFileInfo fileinfo)
{
    int tabIdx = 0;
    QIcon icon(":/icons/service_config.png");
    ConfigFileWidget* widget = new ConfigFileWidget(fileinfo);

    tabIdx = ui->logTabWidget->addTab(widget, icon, fileinfo.fileName());
    ui->logTabWidget->setCurrentIndex(tabIdx);
}

void MainWindow::openLogFile(QFileInfo fileinfo)
{
    int tabIdx = 0;
    QIcon icon(":/icons/service_log.png");
    LogFileWidget* widget = new LogFileWidget(fileinfo.absoluteFilePath());

    tabIdx = ui->logTabWidget->addTab(widget, icon, fileinfo.fileName());
    ui->logTabWidget->setCurrentIndex(tabIdx);
}

void MainWindow::setupServiceWidgets()
{
    std::vector<ServiceDesc_t> svcDescs = ManagerConfigurationManager::getInstance()->getServices();

    foreach (ServiceDesc_t svcDesc, svcDescs) {
        ServiceWidget* svcWidget = new ServiceWidget(svcDesc);

        serviceWidgets.push_back(svcWidget);
        ui->servicesLayout->addWidget(svcWidget);

        connect(svcWidget, SIGNAL(openConfigFile(QFileInfo)),
                this, SLOT(openConfigFile(QFileInfo)));
        connect(svcWidget, SIGNAL(openLogFile(QFileInfo)),
                this, SLOT(openLogFile(QFileInfo)));
    }

    ui->servicesLayout->addStretch();
}
