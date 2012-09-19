#include <QByteArray>
#include "ConfigFileWidget.h"
#include "ui_ConfigFileWidget.h"

ConfigFileWidget::ConfigFileWidget(const QFileInfo& fileinfo, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigFileWidget),
    file(fileinfo.absoluteFilePath())
{
    ui->setupUi(this);

    if (!file.open(QIODevice::ReadWrite)) {
        ui->textBrowser->setPlainText(tr("Failed to open config file (%1): %2").arg(fileinfo.absoluteFilePath()).arg(file.errorString()));
        return;
    }

    QByteArray contents = file.readAll();
    ui->textBrowser->setPlainText(QString(contents));
}

ConfigFileWidget::~ConfigFileWidget()
{
    delete ui;
}
