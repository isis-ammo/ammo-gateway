#include <QFile>
#include <QFileSystemWatcher>
#include "LogFileWidget.h"
#include "ui_LogFileWidget.h"

LogFileWidget::LogFileWidget(const QString& logFileName, QWidget *parent) :
    QWidget(parent),
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
