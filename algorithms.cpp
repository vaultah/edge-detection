#include <QtWidgets>
#include <cmath>
#include <utility>
#include <queue>
#include <vector>
#include "kernels.h"


using namespace std;


matrix gaussian(float sigma) {
    matrix gauss(5, matrix::value_type(5));
    float sum = 0, s = 2 * pow(sigma, 2);

    for (int x = -2; x <= 2; x++)
        for (int y = -2; y <= 2; y++)
            sum += (gauss[x + 2][y + 2] = exp(-(pow(x, 2) + pow(y, 2)) / s) / s / M_PI);

    for (auto& row : gauss)
        for (auto& x : row)
            x /= sum;

    return gauss;
}


void magnitude(QImage& input, const QImage& gx, const QImage& gy) {
    float m;
    for (int x = 0; x < input.width(); x++) {
        for (int y = 0; y < input.height(); y++) {
            m = hypot(qRed(gx.pixel(x, y)), qRed(gy.pixel(x, y)));
            m = qBound(0.f, m, 255.f);
            input.setPixel(x, y, QColor(m, m, m).rgb());
        }
    }
}


QImage convolution(const matrix& kernel, const QImage& image) {
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


QImage hysteresis(const QImage& image, float tmin, float tmax) {
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


QImage canny(const QImage& input, float sigma, float tmin, float tmax) {
    QImage res;

    // Gaussian blur
    res = convolution(gaussian(sigma), input);

    // Gradients
    auto gx = convolution(sobelx, res);
    auto gy = convolution(sobely, res);

    magnitude(res, gx, gy);

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


QImage sobel(const QImage& input) {
    QImage res(input.size(), input.format());
    magnitude(res, convolution(sobelx, input), convolution(sobely, input));
    return res;
}


QImage prewitt(const QImage& input) {
    QImage res(input.size(), input.format());
    magnitude(res, convolution(prewittx, input), convolution(prewitty, input));
    return res;
}

QImage roberts(const QImage& input) {
    QImage res(input.size(), input.format());
    magnitude(res, convolution(robertsx, input), convolution(robertsy, input));
    return res;
}


QImage scharr(const QImage& input) {
    QImage res(input.size(), input.format());
    magnitude(res, convolution(scharrx, input), convolution(scharry, input));
    return res;
}
