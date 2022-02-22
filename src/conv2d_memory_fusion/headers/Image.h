#ifndef CONV2D_IMAGE_H
#define CONV2D_IMAGE_H

#include "Matrix3D.h"
#include "PDBObject.h"

#include <iostream>

namespace conv2d_memory_fusion {
class Image : public Matrix3D {
public:
  int index;

  ENABLE_DEEP_COPY
  Image() {}
  Image(int index, int x, int y, int z) : index(index), Matrix3D(x, y, z) {}

  int &getKey() { return index; }

  Image &getValue() { return *this; }

  int get_x(int padding) { return x + padding * 2; }

  int get_y(int padding) { return y + padding * 2; }

  int get_h_slides(int padding, int kernel_size, int stride) {
    return floor((get_y(padding) - kernel_size) / stride) + 1;
  }

  int get_v_slides(int padding, int kernel_size, int stride) {
    return floor((get_x(padding) - kernel_size) / stride) + 1;
  }

  // TODO: odd Padding, odd strides, odd kernels
  // Maybe another class that can save state and operate on this img?
  float get_win_value(int window, int i, int j, int stride, int channel,
                       int kernel_size, int padding) {
    int h_slides = get_h_slides(padding, kernel_size, stride);
    int v_slides = get_v_slides(padding, kernel_size, stride);

    // calculate window origin
    int r = floor(window / h_slides);
    int c = window % h_slides;

    // Find where i,j lies in the kernel window
    // Also adjust for padding
    // We assume that padding is out of bounds of the actual array
    int x_index = ((r * stride) + i) - padding;
    int y_index = ((c * stride) + j) - padding;

    if (padding > 0 &&
        (x_index < 0 || y_index < 0 || x_index >= x || y_index >= y))
      return 0;

    int flat_index = x_index * y + y_index;
    return matrices[channel]->getValue().rawData->c_ptr()[flat_index];
  }

  int get_conv2d_matrix_rows(int kernel_size, int stride, int padding) {
    int v_slides = get_v_slides(padding, kernel_size, stride);
    int h_slides = get_h_slides(padding, kernel_size, stride);

    return v_slides * h_slides;
  }

  int get_conv2d_matrix_cols(int kernel_size, int stride, int padding) {
    return z * kernel_size * kernel_size;
  }

  int get_conv2d_window_count(int kernel_size, int stride, int padding) {
    int h_slides = get_h_slides(padding, kernel_size, stride);
    int v_slides = get_v_slides(padding, kernel_size, stride);
    std::cout << "[IMAGE] h_slides: " << h_slides << ", v_slides: " << v_slides << std::endl;
    std::cout << "[IMAGE] kernel: " << kernel_size << ", strides: " << stride << ", padding: " << padding << std::endl;

    return h_slides * v_slides;
  }

  int get_num_channels() { return z; }

  Image &operator+(Image &addMeIn) {
    for (int c = 0; c < z; c++) {
      // assert((getMatrixAtIndex(c) == nullptr && addMeIn.getMatrixAtIndex(c) != nullptr) || (getMatrixAtIndex(c) != nullptr && addMeIn.getMatrixAtIndex(c) == nullptr));
      if (getMatrixAtIndex(c) == nullptr) {
        setMatrixAtIndex(c, addMeIn.getMatrixAtIndex(c));
      }
    }

    return *this;
  }
};
} // namespace conv2d_memory_fusion

#endif
