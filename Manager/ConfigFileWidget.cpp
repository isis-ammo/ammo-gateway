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
