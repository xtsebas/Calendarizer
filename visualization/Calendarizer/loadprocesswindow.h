#ifndef LOADPROCESSWINDOW_H
#define LOADPROCESSWINDOW_H

#include <QDialog>

namespace Ui {
class LoadProcessWindow;
}

class LoadProcessWindow : public QDialog {
    Q_OBJECT

public:
    explicit LoadProcessWindow(QWidget *parent = nullptr);
    ~LoadProcessWindow();

private:
    Ui::LoadProcessWindow *ui;
};

#endif // LOADPROCESSWINDOW_H
