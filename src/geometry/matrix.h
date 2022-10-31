#ifndef GEOMETRY_MATRIX_H_
#define GEOMETRY_MATRIX_H_

#include <initializer_list>
#include <vector>

namespace bfg {

template <typename T>
class Matrix {
 public:
  Matrix(const std::vector<std::vector<T>> &elements)
      : elements_(elements) {
    num_rows_ = elements.size();
    num_columns_ = 0;
    for (const auto& row : elements) {
      num_columns_ = std::max(num_columns_, row.size());
    }
  }

  // Read an array like 1 2 3 4 into the matrix:
  // [ 1 2;
  //   3 4 ]
  Matrix(T array[], size_t num_rows, size_t num_columns) {
    num_rows_ = num_rows;
    num_columns_ = num_columns;
    elements_ = std::vector(num_rows, std::vector(num_columns));
    for (size_t i = 0; i < num_rows; ++i) {
      for (size_t j = 0; j < num_columns; ++j) {
        elements_[i][j] = array[i * num_columns + j];
      }
    }
  }

  Matrix(std::initializer_list<std::vector<T>> init)
    : Matrix(std::vector<std::vector<T>>(init)) {}

  Matrix(size_t rows, size_t columns) {
    num_rows_ = rows;
    num_columns_ = columns;
    elements_ = std::vector<std::vector<T>>(columns, std::vector<T>(rows));
  }

  const std::vector<T> &operator[](size_t row_index) {
    return elements_[row_index];
  }

  size_t num_rows() const { return num_rows_; }
  size_t num_columns() const { return num_columns_; }

 private:
  size_t num_rows_;
  size_t num_columns_;

  // Elements in row-major order. The outer vector is a vector of rows.
  std::vector<std::vector<T>> elements_;
};

}  // namespace bfg

#endif  // GEOMETRY_MATRIX_H_
