#ifndef SYNC_LEGEND_H
#define SYNC_LEGEND_H

#include <QWidget>

class SyncLegend : public QWidget {
    Q_OBJECT

public:
    explicit SyncLegend(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif
//dieguiot
