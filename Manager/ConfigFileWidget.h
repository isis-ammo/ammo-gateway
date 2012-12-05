#ifndef CONFIGFILEWIDGET_H
#define CONFIGFILEWIDGET_H

#include <QFile>
#include <QFileInfo>
#include "TabWidget.h"

namespace Ui {
class ConfigFileWidget;
}

class ConfigFileWidget : public TabWidget
{
    Q_OBJECT
    
public:
    explicit ConfigFileWidget(const QFileInfo& file, QWidget *parent = 0);
    ~ConfigFileWidget();

private slots:
    void textChanged();
    void save();

private:
    Ui::ConfigFileWidget *ui;
    QFile file;
};

#endif // CONFIGFILEWIDGET_H
