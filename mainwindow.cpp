#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include <QColor>
#include "kernels.h"
#include <cmath>
#include <utility>
#include <queue>

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

QImage MainWindow::convolution(const matrix& kernel, const QImage& image) {
    int kw = kernel[0].size(), kh = kernel.size(),
        offsetx = kw / 2, offsety = kw / 2;
    QImage out(image.size(), image.format());
    float sum;

    for (int x = 0; x < image.width(); x++) {
        for (int y = 0; y < image.height(); y++) {
            sum = 0;

            for (int i = 0; i < kw; i++) {
                for (int j = 0; j < kh; j++) {
                    if (y + j < offsety || x + i < offsetx
                        || y + j >= image.height() || x + i >= image.width())
                        continue;

                    auto pixel = image.pixel(x + i - offsetx, y + j - offsety);
                    sum += kernel[j][i] * qRed(pixel);
                }
            }

            sum = qBound(0.f, sum, 255.f);
            out.setPixel(x, y, QColor(sum, sum, sum).rgb());
        }
    }

    return out;
}

QImage MainWindow::canny(const QImage& input, float tmin, float tmax) {
    // Gaussian blur
    matrix gauss {
        {2,  4,  5,  4,  2},
        {4,  9, 12,  9,  4},
        {5, 12, 15, 12,  5},
        {4,  9, 12,  9,  4},
        {2,  4,  5,  4,  2}
    };

    QImage res;
    float g;

    for (auto& row : gauss)
        for (auto& x : row)
            x /= 159;

    res = convolution(gauss, input);

    // Gradients
    auto gx = convolution(sobelx, res);
    auto gy = convolution(sobely, res);

    for (int x = 0; x < res.width(); x++) {
        for (int y = 0; y < res.height(); y++) {
            g = hypot(qRed(gx.pixel(x, y)), qRed(gy.pixel(x, y)));
            g = qBound(0.f, g, 255.f);
            res.setPixel(x, y, QColor(g, g, g).rgb());
        }
    }

    // Non-maximum suppression
    auto cmp = [](QRgb p, QRgb c, QRgb n) -> bool {
        return qRed(p) < qRed(c) && qRed(n) < qRed(c);
    };

    for (int x = 1; x < res.width() - 1; x++) {
        for (int y = 1; y < res.height() - 1; y++) {
            auto at = atan2(qRed(gy.pixel(x, y)), qRed(gx.pixel(x, y)));
            const auto dir = fmod(at + M_PI, M_PI) / M_PI * 8;
            auto p = res.pixel(x, y);
            if ((1 >= dir || dir > 7) && cmp(res.pixel(x - 1, y), p, res.pixel(x + 1, y))
                || (1 < dir || dir <= 3) && cmp(res.pixel(x - 1, y - 1), p, res.pixel(x + 1, y + 1))
                || (3 < dir || dir <= 5) && cmp(res.pixel(x, y - 1), p, res.pixel(x, y + 1))
                || (5 < dir || dir <= 7) && cmp(res.pixel(x + 1, y - 1), p, res.pixel(x - 1, y + 1)))
                res.setPixel(x, y, p);
            else res.setPixel(x, y, QColor(Qt::black).rgb());
        }
    }

    // Hysteresis
    return hysteresis(res, tmin, tmax);
}

QImage MainWindow::hysteresis(const QImage& image, float tmin, float tmax) {
    auto res = QImage(image.size(), image.format());
    res.fill(Qt::black);
    queue<pair<int, int>> edges;
    float ni, nj;

    for (int x = 1; x < res.width() - 1; x++) {
        for (int y = 1; y < res.height() - 1; y++) {
            if (qRed(image.pixel(x, y)) >= tmax && qRed(res.pixel(x, y)) == 0) {
                res.setPixel(x, y, QColor(Qt::white).rgb());
                edges.push(make_pair(x, y));

                while (!edges.empty()) {
                    auto point = edges.front();
                    edges.pop();

                    for (int i = -1; i <= 1; i++) {
                        for (int j = -1; j <= 1; j++) {
                            ni = point.first + i;
                            nj = point.second + j;
                            if (!i && !j || ni < 0 || nj < 0 || ni >= image.width() || nj >= image.height())
                                continue;

                            if (qRed(image.pixel(ni, nj)) >= tmin && qRed(res.pixel(ni, nj)) == 0) {
                                res.setPixel(ni, nj, QColor(Qt::white).rgb());
                                edges.push(make_pair(ni, nj));
                            }
                        }
                    }
                }

            }
        }
    }

    return res;
}

QImage MainWindow::sobel(const QImage& input) {
    QImage res(input.size(), input.format());
    QImage gx = convolution(sobelx, input), gy = convolution(sobely, input);
    float i;

    for (int x = 0; x < res.width(); x++) {
        for (int y = 0; y < res.height(); y++) {
            i = hypot(qRed(gx.pixel(x, y)), qRed(gy.pixel(x, y)));
            i = qBound(0.f, i, 255.f);
            res.setPixel(x, y, QColor(i, i, i).rgb());
        }
    }

    return res;
}

QImage MainWindow::prewitt(const QImage& input) {
    QImage res(input.size(), input.format());
    QImage gx = convolution(prewittx, input), gy = convolution(prewitty, input);
    float i;

    for (int x = 0; x < res.width(); x++) {
        for (int y = 0; y < res.height(); y++) {
            i = hypot(qRed(gx.pixel(x, y)), qRed(gy.pixel(x, y)));
            i = qBound(0.f, i, 255.f);
            res.setPixel(x, y, QColor(i, i, i).rgb());
        }
    }

    return res;
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
