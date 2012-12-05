#include "TabWidget.h"

TabWidget::TabWidget(QWidget *parent) :
    QWidget(parent),
    _dirty(false)
{
}

TabWidget::~TabWidget()
{
}

void TabWidget::setDirty(bool isDirty)
{
    if (isDirty == this->_dirty) return;

    this->_dirty = isDirty;
    emit dirty(this, this->_dirty);
}
