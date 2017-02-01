#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "algorithms.h"


using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    auto file = QFileDialog::getOpenFileName(this, tr("Select a single image"));
    original = QImage(file);
    display(original);
    grayscale = original.convertToFormat(QImage::Format_Grayscale8);
    ui->comboBox->setCurrentIndex(0);
}


void MainWindow::display(const QImage& image) {
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(image));
    scene->setSceneRect(image.rect());
    ui->image->setScene(scene);
    ui->image->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        display(original);
        break;
    case 1:
        display(canny(grayscale, 40, 50));
        break;
    case 2:
        display(sobel(grayscale));
        break;
    case 3:
        display(prewitt(grayscale));
        break;
    default:
        break;
    }
}


void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    ui->image->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}
