/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */

#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QVector>
#include "AboutDialog.h"
#include "ConfigFileWidget.h"
#include "ManagerConfigurationManager.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "LogFileWidget.h"
#include "ServiceDesc.h"
#include "TabWidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->addAction(ui->action_Open_Config_File);
    ui->mainToolBar->addAction(ui->action_Open_Log_File);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionSave);
    ui->mainToolBar->addAction(ui->actionSave_All);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionExit);
    setupServiceWidgets();

    connect(ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(about()));
    connect(ui->action_Open_Config_File, SIGNAL(triggered()),
            this, SLOT(openConfig()));
    connect(ui->action_Open_Log_File, SIGNAL(triggered()),
            this, SLOT(openLog()));
    connect(ui->actionSave, SIGNAL(triggered()),
            this, SLOT(save()));
    connect(ui->actionExit, SIGNAL(triggered()),
            this, SLOT(exit()));

    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeLogTab(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::about()
{
    AboutDialog* about = new AboutDialog(this);
    about->exec();
}

void MainWindow::save()
{
    TabWidget* w = static_cast<TabWidget*>(ui->tabWidget->currentWidget());
    if (!w) return;

    w->save();
}

void MainWindow::saveAll()
{
    for (int i = 0; i < ui->tabWidget->count(); i++) {
        TabWidget* w = static_cast<TabWidget*>(ui->tabWidget->widget(i));
        if (!w) continue;

        w->save();
    }
}

void MainWindow::exit()
{
    QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel;
    bool saveAll = false;
    bool saveNone = false;

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        TabWidget* w = static_cast<TabWidget*>(ui->tabWidget->widget(i));
        if (!w) continue;

        if (!w->getDirty()) continue;

        if (saveAll) {
            w->save();
            continue;
        }

        if (saveNone) {
            continue;
        }

        QMessageBox::StandardButton b;
        b = QMessageBox::question(this,
                                  "Save Changes?",
                                  QString("%1 has unsaved changes. Save?").arg(w->getName()),
                                  buttons,
                                  QMessageBox::Cancel);

        switch (b) {
        case QMessageBox::YesToAll:
            saveAll = true;
            // fall through to Yes
        case QMessageBox::Yes:
            w->save();
            break;

        case QMessageBox::NoToAll:
            saveNone = true;
            // fall through to No
        case QMessageBox::No:
            break;

        case QMessageBox::Cancel:
            return;
        }
    }

    close();
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
    QWidget* logWidget = ui->tabWidget->widget(idx);

    if (logWidget) {
        ui->tabWidget->removeTab(idx);
        delete logWidget;
    }
}

void MainWindow::openConfigFile(QFileInfo fileinfo)
{
    int tabIdx = 0;
    QIcon icon(":/icons/service_config.png");
    ConfigFileWidget* widget = new ConfigFileWidget(fileinfo);

    tabIdx = ui->tabWidget->addTab(widget, icon, fileinfo.fileName());
    widget->setName(fileinfo.fileName());
    ui->tabWidget->setCurrentIndex(tabIdx);

    connect(widget, SIGNAL(dirty(TabWidget*,bool)),
            this, SLOT(dirtyTab(TabWidget*,bool)));
}

void MainWindow::openLogFile(QFileInfo fileinfo)
{
    int tabIdx = 0;
    QIcon icon(":/icons/service_log.png");
    LogFileWidget* widget = new LogFileWidget(fileinfo.absoluteFilePath());

    tabIdx = ui->tabWidget->addTab(widget, icon, fileinfo.fileName());
    widget->setName(fileinfo.fileName());
    ui->tabWidget->setCurrentIndex(tabIdx);

    connect(widget, SIGNAL(dirty(TabWidget*,bool)),
            this, SLOT(dirtyTab(TabWidget*,bool)));
}

void MainWindow::dirtyTab(TabWidget *w, bool dirty)
{
    int idx = ui->tabWidget->indexOf(w);
    if (idx == -1) return;

    QString text = dirty ? w->getName() + "*" : w->getName();

    ui->tabWidget->setTabText(idx, text);
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

        QMenu* menu = new QMenu(svcDesc.humanName.c_str());
        ui->menuServices->addMenu(menu);

        foreach (QAction* action, svcWidget->actions()) {
            menu->addAction(action);
        }
    }

    ui->servicesLayout->addStretch();
}
