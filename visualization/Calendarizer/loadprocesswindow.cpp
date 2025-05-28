#include "loadprocesswindow.h"
#include "ui_loadprocesswindow.h"

LoadProcessWindow::LoadProcessWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoadProcessWindow)
{
    ui->setupUi(this);
}

LoadProcessWindow::~LoadProcessWindow()
{
    delete ui;
}
