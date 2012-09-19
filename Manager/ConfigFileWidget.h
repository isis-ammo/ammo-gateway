#ifndef CONFIGFILEWIDGET_H
#define CONFIGFILEWIDGET_H

#include <QFile>
#include <QFileInfo>
#include <QWidget>

namespace Ui {
class ConfigFileWidget;
}

class ConfigFileWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ConfigFileWidget(const QFileInfo& file, QWidget *parent = 0);
    ~ConfigFileWidget();
    
private:
    Ui::ConfigFileWidget *ui;
    QFile file;
};

#endif // CONFIGFILEWIDGET_H
