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

#ifndef SERVICEWIDGET_H
#define SERVICEWIDGET_H

#include <QFileInfo>
#include <QWidget>
#include <QVector>
#include "ServiceDesc.h"

namespace Ui {
class ServiceWidget;
}

class ServiceController;

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
    void update();

private:
    Ui::ServiceWidget *ui;
    ServiceDesc_t desc;
    ServiceController* ctrl;
    QAction* actionStart;
    QAction* actionStop;
    QAction* actionRestart;
    QAction* actionConfig;
    QAction* actionLog;
};

#endif // SERVICEWIDGET_H
