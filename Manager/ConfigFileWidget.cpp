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

#include <QByteArray>
#include <QMessageBox>
#include "ConfigFileWidget.h"
#include "ui_ConfigFileWidget.h"

ConfigFileWidget::ConfigFileWidget(const QFileInfo& fileinfo, QWidget *parent) :
    TabWidget(parent),
    ui(new Ui::ConfigFileWidget),
    file(fileinfo.absoluteFilePath())
{
    ui->setupUi(this);

    if (!file.open(QIODevice::ReadWrite)) {
        ui->textBrowser->setPlainText(tr("Failed to open config file (%1): %2").arg(fileinfo.absoluteFilePath()).arg(file.errorString()));
        ui->textBrowser->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->saveButton->setEnabled(false);
        return;
    }

    QByteArray contents = file.readAll();
    ui->textBrowser->setPlainText(QString(contents));

    connect(ui->textBrowser, SIGNAL(textChanged()),
            this, SLOT(textChanged()));
    connect(ui->saveButton, SIGNAL(clicked()),
            this, SLOT(save()));
}

ConfigFileWidget::~ConfigFileWidget()
{
    delete ui;
}

void ConfigFileWidget::textChanged()
{
    setDirty(true);
}

void ConfigFileWidget::save()
{
    if (!file.resize(0)) {
        QMessageBox::warning(this, "Unable To Save File", QString("Unable to resize %1 during save.").arg(file.errorString()));
        return;
    }
    if (!file.write(ui->textBrowser->toPlainText().toLocal8Bit())) {
        QMessageBox::warning(this, "Unable To Save File", QString("Unable to write %1 during save.").arg(file.errorString()));
        return;
    }
    file.flush();

    setDirty(false);
}
