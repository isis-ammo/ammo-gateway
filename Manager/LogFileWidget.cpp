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

#include <QFile>
#include <QFileSystemWatcher>
#include "LogFileWidget.h"
#include "ui_LogFileWidget.h"

LogFileWidget::LogFileWidget(const QString& logFileName, QWidget *parent) :
    TabWidget(parent),
    ui(new Ui::LogFileWidget)
{
    ui->setupUi(this);

    file = new QFile(logFileName);
    if (!file->open(QIODevice::ReadOnly)) {
        ui->textBrowser->setPlainText(tr("Failed to open log file (%1): %2").arg(logFileName).arg(file->errorString()));
    }

    connect(&watcher, SIGNAL(fileChanged(QString)),
            this, SLOT(moreDataAvailable(QString)));
    watcher.addPath(logFileName);

    QByteArray contents = file->readAll();
    ui->textBrowser->setPlainText(QString(contents));

    // Scroll to the end
    QTextCursor autoScrollCursor = ui->textBrowser->textCursor();
    autoScrollCursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(autoScrollCursor);
    ui->textBrowser->ensureCursorVisible();
}

LogFileWidget::~LogFileWidget()
{
    file->close();
    delete ui;
    delete file;
}

void LogFileWidget::moreDataAvailable(const QString&)
{
    QTextBrowser* browser = ui->textBrowser;
    QString newContent;
    QByteArray data;

    // Read new content from end of log file. Note: readLine() does not throw an error on failure.
    // Instead, it returns an empty byte array. So if data is empty  and bytes are available, you
    // have entered an error condition when reading the file. For now we'll handle this by ignoring
    // it which will cause nothing to be added to the screen.
    do {
        data = file->readLine(75);
        newContent.append(data);
    } while (!data.isEmpty() && file->bytesAvailable() > 0);

    // Insert the text at the end of the document.
    QTextCursor insertionCursor(browser->document());
    insertionCursor.movePosition(QTextCursor::End);
    insertionCursor.beginEditBlock();
    insertionCursor.insertText(newContent);
    insertionCursor.endEditBlock();

    // Autoscroll to the end if enabled.
    if (ui->autoScrollCheckBox->isChecked()) {
        QTextCursor autoScrollCursor = browser->textCursor();
        autoScrollCursor.movePosition(QTextCursor::End);
        browser->setTextCursor(autoScrollCursor);
        browser->ensureCursorVisible();
    }
}
