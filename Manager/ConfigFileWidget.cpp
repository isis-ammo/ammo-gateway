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
