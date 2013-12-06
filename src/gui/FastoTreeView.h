#pragma once

#include <QTreeView>

namespace fastoredis
{
    class FastoTreeView
            : public QTreeView
    {        
        Q_OBJECT
    public:
        typedef QTreeView base_class;
        FastoTreeView(QWidget *parent = 0);

    private Q_SLOTS:
        void showContextMenu(const QPoint &point);
    };
}
