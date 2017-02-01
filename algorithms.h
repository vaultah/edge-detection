#ifndef ALGORITHMS_H
#define ALGORITHMS_H

QImage convolution(const matrix&, const QImage&);
QImage canny(const QImage&, float, float);
QImage sobel(const QImage&);
QImage prewitt(const QImage&);
QImage hysteresis(const QImage&, float, float);

#endif // ALGORITHMS_H
