#ifndef KERNELS_H
#define KERNELS_H
#include <vector>

typedef std::vector<std::vector<float>> matrix;
const matrix sobelx {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const matrix sobely {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

#endif // KERNELS_H
