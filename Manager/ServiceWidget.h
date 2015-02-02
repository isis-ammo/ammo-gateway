/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
