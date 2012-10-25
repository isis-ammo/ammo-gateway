#include <QByteArray>
#include <QResource>
#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <QMessageBox>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->version->setText("Version: " + readVersion());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

QString AboutDialog::readVersion()
{
    QResource versionResource(":/version.txt");
    QByteArray bytes(reinterpret_cast<const char*>(versionResource.data()), versionResource.size());
    return QString(bytes);
}
