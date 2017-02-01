#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QtWidgets>
#include "kernels.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QImage original, grayscale;
    QGraphicsScene *scene;

    void display(const QImage&);
    void resizeEvent(QResizeEvent*);
};


#endif // MAINWINDOW_H
