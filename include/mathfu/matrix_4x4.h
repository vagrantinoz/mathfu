/*
* Copyright 2014 Google Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef MATHFU_MATRIX_4X4_H_
#define MATHFU_MATRIX_4X4_H_

#include "mathfu/matrix.h"
#include "mathfu/vector_4.h"
#include "mathfu/utilities.h"

#ifdef COMPILE_WITH_SIMD
#include "vectorial/simd4x4f.h"
#endif

namespace mathfu {

#ifdef COMPILE_WITH_SIMD
template<>
class Matrix<float, 4> {
 public:
  Matrix<float, 4>() {}

  inline Matrix<float, 4>(const Matrix<float, 4>& m) {
    data_.x = m.data_.x; data_.y = m.data_.y;
    data_.z = m.data_.z; data_.w = m.data_.w;
  }

  explicit inline Matrix<float, 4>(const float& s) {
    simd4f v = simd4f_create(s, s, s, s);
    data_ = simd4x4f_create(v, v, v, v);
  }

  inline Matrix<float, 4>(
    const float& s00, const float& s10, const float& s20, const float& s30,
    const float& s01, const float& s11, const float& s21, const float& s31,
    const float& s02, const float& s12, const float& s22, const float& s32,
    const float& s03, const float& s13, const float& s23, const float& s33) {
    data_ = simd4x4f_create(
      simd4f_create(s00, s10, s20, s30),
      simd4f_create(s01, s11, s21, s31),
      simd4f_create(s02, s12, s22, s32),
      simd4f_create(s03, s13, s23, s33));
  }

  explicit inline Matrix<float, 4>(const float* m) {
    data_ = simd4x4f_create(
      simd4f_create(m[0], m[1], m[2], m[3]),
      simd4f_create(m[4], m[5], m[6], m[7]),
      simd4f_create(m[8], m[9], m[10], m[11]),
      simd4f_create(m[12], m[13], m[14], m[15]));
  }

  inline const float& operator()(const int i, const int j) const {
    return FindElem(i, FindColumn(j));
  }

  inline const float& operator()(const int i) const {
    const int col = (i - 1) / 4;
    const int row = (i - 1) % 4;
    return FindElem(row, FindColumn(col));
  }

  inline const float& operator[](const int i) const {
    const int col = i / 4;
    const int row = i % 4;
    return FindElem(row, FindColumn(col));
  }

  inline Matrix<float, 4> operator-() const {
    Matrix<float, 4> m(0.f);
    simd4x4f_sub(&m.data_, &data_, &m.data_);
    return m;
  }

  inline Matrix<float, 4> operator+(const Matrix<float, 4>& m) const {
    Matrix<float, 4> return_m;
    simd4x4f_add(&data_, &m.data_, &return_m.data_);
    return return_m;
  }

  inline Matrix<float, 4> operator-(const Matrix<float, 4>& m) const {
    Matrix<float, 4> return_m;
    simd4x4f_sub(&data_, &m.data_, &return_m.data_);
    return return_m;
  }

  inline Matrix<float, 4> operator*(const float& s) const {
    Matrix<float, 4> m(s);
    simd4x4f_mul(&m.data_, &data_, &m.data_);
    return m;
  }

  inline Matrix<float, 4> operator/(const float& s) const {
    Matrix<float, 4> m(1 / s);
    simd4x4f_mul(&m.data_, &data_, &m.data_);
    return m;
  }

  inline Vector<float, 3> operator*(const Vector<float, 3>& v) const {
    Vector<float, 3> return_v;
#ifdef COMPILE_WITH_PADDING
    v.data_[3] = 1;
    simd4x4f_matrix_vector_mul(&data_, &v.data_, &return_v.data_);
    return_v *= (1 / return_v.data_[3]);
#else
    simd4f vec = MATHFU_LOAD(v.data_);
    simd4x4f_matrix_vector_mul(&data_, &vec, &vec);
    simd4f_mul(vec, simd4f_splat(*(MATHFU_CAST<float*>(&vec) + 3)));
    MATHFU_STORE(vec, return_v.data_);
#endif
    return return_v;
  }

  inline Vector<float, 4> operator*(const Vector<float, 4>& v) const {
    Vector<float, 4> return_v;
    simd4x4f_matrix_vector_mul(&data_, &v.data_, &return_v.data_);
    return return_v;
  }

  inline Vector<float, 4> VecMatTimes(const Vector<float, 4>& v) const {
    return Vector<float, 4>(
      simd4f_dot3(v.data_, data_.x),
      simd4f_dot3(v.data_, data_.y),
      simd4f_dot3(v.data_, data_.w),
      simd4f_dot3(v.data_, data_.z));
  }

  inline Matrix<float, 4> operator*(const Matrix<float, 4>& m) const {
    Matrix<float, 4> return_m;
    simd4x4f_matrix_mul(&data_, &m.data_, &return_m.data_);
    return return_m;
  }

  Matrix<float, 4> Inverse() const {
    Matrix<float, 4> return_m;
    simd4x4f_inverse(&data_, &return_m.data_);
    return return_m;
  }

  inline Vector<float, 3> TranslationVector3D() const {
    Vector<float, 3> return_v;
    MATHFU_STORE(FindColumn(3), return_v.data_);
    return return_v;
  }

  inline Matrix<float, 4>& operator+=(const Matrix<float, 4>& m) {
    simd4x4f_add(&data_, &m.data_, &data_);
    return *this;
  }

  inline Matrix<float, 4>& operator-=(const Matrix<float, 4>& m) {
    simd4x4f_sub(&data_, &m.data_, &data_);
    return *this;
  }

  inline Matrix<float, 4>& operator*=(const float& s) {
    Matrix<float, 4> m(s);
    simd4x4f_mul(&m.data_, &data_, &data_);
    return *this;
  }

  inline Matrix<float, 4>& operator/=(const float& s) {
    Matrix<float, 4> m(1 / s);
    simd4x4f_mul(&m.data_, &data_, &data_);
    return *this;
  }

  inline Matrix<float, 4> operator*=(const Matrix<float, 4>& m) {
    simd4x4f_matrix_mul(&data_, &m.data_, &data_);
    return *this;
  }

  static inline Matrix<float, 4> OuterProduct(
    const Vector<float, 4>& v1, const Vector<float, 4>& v2) {
    Matrix<float, 4> m;
    m.data_ = simd4x4f_create(
      simd4f_mul(v1.data_, simd4f_splat(v2[0])),
      simd4f_mul(v1.data_, simd4f_splat(v2[1])),
      simd4f_mul(v1.data_, simd4f_splat(v2[2])),
      simd4f_mul(v1.data_, simd4f_splat(v2[3])));
    return m;
  }

  static inline Matrix<float, 4> HadamardProduct(
    const Matrix<float, 4>& m1, const Matrix<float, 4>& m2) {
    Matrix<float, 4> return_m;
    simd4x4f_mul(&m1.data_, &m2.data_, &return_m.data_);
    return return_m;
  }

  static inline Matrix<float, 4> Identity() {
    Matrix<float, 4> return_m;
    simd4x4f_identity(&return_m.data_);
    return return_m;
  }

  static inline Matrix<float, 4> FromTranslationVector(const Vector<float, 3>& v) {
    return Matrix<float, 4>(
      1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, v[0], v[1], v[2], 1);
  }

  static inline Matrix<float, 4> FromRotationMatrix(const Matrix<float, 3>& m) {
    return Matrix<float, 4>(
      m[0], m[1], m[2], 0, m[3], m[4], m[5], 0,
      m[6], m[7], m[8], 0, 0, 0, 0, 1);
  }

 private:
  inline const simd4f& FindColumn(const int i) const {
    return *(MATHFU_CAST<simd4f*>(&data_) + i);
  }

  inline const float& FindElem(const int i, const simd4f& column) const {
    return *(MATHFU_CAST<float*>(&column) + i);
  }

  simd4x4f data_;
};

inline Matrix<float, 4> operator*(const float& s, const Matrix<float, 4>& m) {
  return m * s;
}

inline Vector<float, 4> operator*(
  const Vector<float, 4>& v, const Matrix<float, 4>& m) {
  return m.VecMatTimes(v);
}
#endif  // COMPILE_WITH_SIMD
}  // namespace mathfu

#endif  // MATHFU_MATRIX_4X4_H_
