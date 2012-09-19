#ifndef DIRTYABLEWIDGET_H
#define DIRTYABLEWIDGET_H

#include <QWidget>

class TabWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabWidget(QWidget *parent = 0);
    ~TabWidget();

    void setDirty(bool isDirty);
    bool getDirty() { return _dirty; }

    void setName(const QString& name) { this->_name = name; }
    QString getName() { return this->_name; }

    virtual void save() {}

signals:
    void dirty(TabWidget* w, bool isDirty);

private:
    bool _dirty;
    QString _name;
};

#endif // DIRTYABLEWIDGET_H
