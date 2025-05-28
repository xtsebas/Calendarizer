#include "resultwindow.h"
#include "ui_resultwindow.h"

ResultWindow::ResultWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResultWindow)
{
    ui->setupUi(this);
}

ResultWindow::~ResultWindow()
{
    delete ui;
}
