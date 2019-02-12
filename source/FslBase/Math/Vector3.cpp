/****************************************************************************************************************************************************
 * Copyright (c) 2014 Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *
 *    * Neither the name of the Freescale Semiconductor, Inc. nor the names of
 *      its contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************************************************************************/

// The functions in this file are a port of an MIT licensed library: MonaGame - Vector3.cs.

/*
MIT License
Copyright © 2006 The Mono.Xna Team

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cassert>
#include <FslBase/Exceptions.hpp>
#include <FslBase/Math/MathHelper.hpp>
#include <FslBase/Math/Matrix.hpp>
#include <FslBase/Math/Vector3.hpp>
#include <FslBase/Math/Quaternion.hpp>
#include <cassert>
#include <cmath>
#include <cstddef>
#include "MatrixInternals.hpp"

namespace Fsl
{
  const float* Vector3::DirectAccess() const
  {
    // Verify that our assumption about the structure packing is correct
    assert(offsetof(Vector3, X) == (sizeof(float) * 0));
    assert(offsetof(Vector3, Y) == (sizeof(float) * 1));
    assert(offsetof(Vector3, Z) == (sizeof(float) * 2));
    return &X;
  }


  Vector3 Vector3::Barycentric(const Vector3& value1, const Vector3& value2, const Vector3& value3, const float amount1, const float amount2)
  {
    return Vector3(MathHelper::Barycentric(value1.X, value2.X, value3.X, amount1, amount2),
                   MathHelper::Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2),
                   MathHelper::Barycentric(value1.Z, value2.Z, value3.Z, amount1, amount2));
  }


  void Vector3::Barycentric(const Vector3& value1, const Vector3& value2, const Vector3& value3, const float amount1, const float amount2,
                            Vector3& rResult)
  {
    rResult.X = MathHelper::Barycentric(value1.X, value2.X, value3.X, amount1, amount2);
    rResult.Y = MathHelper::Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2);
    rResult.Z = MathHelper::Barycentric(value1.Z, value2.Z, value3.Z, amount1, amount2);
  }


  Vector3 Vector3::CatmullRom(const Vector3& value1, const Vector3& value2, const Vector3& value3, const Vector3& value4, const float amount)
  {
    return Vector3(MathHelper::CatmullRom(value1.X, value2.X, value3.X, value4.X, amount),
                   MathHelper::CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount),
                   MathHelper::CatmullRom(value1.Z, value2.Z, value3.Z, value4.Z, amount));
  }


  void Vector3::CatmullRom(const Vector3& value1, const Vector3& value2, const Vector3& value3, const Vector3& value4, const float amount,
                           Vector3& rResult)
  {
    rResult.X = MathHelper::CatmullRom(value1.X, value2.X, value3.X, value4.X, amount);
    rResult.Y = MathHelper::CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount);
    rResult.Z = MathHelper::CatmullRom(value1.Z, value2.Z, value3.Z, value4.Z, amount);
  }


  Vector3 Vector3::Clamp(const Vector3& value, const Vector3& min, const Vector3& max)
  {
    return Vector3(MathHelper::Clamp(value.X, min.X, max.X), MathHelper::Clamp(value.Y, min.Y, max.Y), MathHelper::Clamp(value.Z, min.Z, max.Z));
  }


  void Vector3::Clamp(Vector3& rResult, const Vector3& value, const Vector3& min, const Vector3& max)
  {
    rResult = Vector3(MathHelper::Clamp(value.X, min.X, max.X), MathHelper::Clamp(value.Y, min.Y, max.Y), MathHelper::Clamp(value.Z, min.Z, max.Z));
  }


  Vector3 Vector3::Cross(const Vector3& vector1, const Vector3& vector2)
  {
    Vector3 result(OptimizationFlag::NoInitialization);
    Cross(result, vector1, vector2);
    return result;
  }


  void Vector3::Cross(Vector3& rResult, const Vector3& vector1, const Vector3& vector2)
  {
    rResult = Vector3(vector1.Y * vector2.Z - vector2.Y * vector1.Z, -(vector1.X * vector2.Z - vector2.X * vector1.Z),
                      vector1.X * vector2.Y - vector2.X * vector1.Y);
  }


  float Vector3::Distance(const Vector3& vector1, const Vector3& vector2)
  {
    return std::sqrt(DistanceSquared(vector1, vector2));
  }


  Vector3 Vector3::Hermite(const Vector3& value1, const Vector3& tangent1, const Vector3& value2, const Vector3& tangent2, const float amount)
  {
    return Vector3(MathHelper::Hermite(value1.X, tangent1.X, value2.X, tangent2.X, amount),
                   MathHelper::Hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, amount),
                   MathHelper::Hermite(value1.Z, tangent1.Z, value2.Z, tangent2.Z, amount));
  }


  void Vector3::Hermite(const Vector3& value1, const Vector3& tangent1, const Vector3& value2, const Vector3& tangent2, const float amount,
                        Vector3& rResult)
  {
    rResult.X = MathHelper::Hermite(value1.X, tangent1.X, value2.X, tangent2.X, amount);
    rResult.Y = MathHelper::Hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, amount);
    rResult.Z = MathHelper::Hermite(value1.Z, tangent1.Z, value2.Z, tangent2.Z, amount);
  }


  float Vector3::Length() const
  {
    return std::sqrt((X * X) + (Y * Y) + (Z * Z));
  }


  Vector3 Vector3::Lerp(const Vector3& value1, const Vector3 value2, const float amount)
  {
    return Vector3(MathHelper::Lerp(value1.X, value2.X, amount), MathHelper::Lerp(value1.Y, value2.Y, amount),
                   MathHelper::Lerp(value1.Z, value2.Z, amount));
  }


  void Vector3::Lerp(Vector3& rResult, const Vector3& value1, const Vector3 value2, const float amount)
  {
    rResult = Vector3(MathHelper::Lerp(value1.X, value2.X, amount), MathHelper::Lerp(value1.Y, value2.Y, amount),
                      MathHelper::Lerp(value1.Z, value2.Z, amount));
  }


  Vector3 Vector3::Max(const Vector3& value1, const Vector3& value2)
  {
    return Vector3(std::max(value1.X, value2.X), std::max(value1.Y, value2.Y), std::max(value1.Z, value2.Z));
  }


  void Vector3::Max(Vector3& rResult, const Vector3& value1, const Vector3& value2)
  {
    rResult = Vector3(std::max(value1.X, value2.X), std::max(value1.Y, value2.Y), std::max(value1.Z, value2.Z));
  }


  Vector3 Vector3::Min(const Vector3& value1, const Vector3& value2)
  {
    return Vector3(std::min(value1.X, value2.X), std::min(value1.Y, value2.Y), std::min(value1.Z, value2.Z));
  }


  void Vector3::Min(Vector3& rResult, const Vector3& value1, const Vector3& value2)
  {
    rResult = Vector3(std::min(value1.X, value2.X), std::min(value1.Y, value2.Y), std::min(value1.Z, value2.Z));
  }


  void Vector3::Negate(Vector3& rResult, const Vector3& value)
  {
    rResult.X = -value.X;
    rResult.Y = -value.Y;
    rResult.Z = -value.Z;
  }


  void Vector3::Normalize()
  {
    Normalize(*this, *this);
  }


  Vector3 Vector3::Normalize(const Vector3& vector)
  {
    Vector3 result(OptimizationFlag::NoInitialization);
    Normalize(result, vector);
    return result;
  }


  void Vector3::Normalize(Vector3& rResult, const Vector3& value)
  {
    float factor = value.Length();
    assert(factor != 0.0f);
    factor = 1.0f / factor;
    rResult.X = value.X * factor;
    rResult.Y = value.Y * factor;
    rResult.Z = value.Z * factor;
  }


  Vector3 Vector3::Reflect(const Vector3& vector, const Vector3& normal)
  {
    // I is the original array
    // N is the normal of the incident plane
    // R = I - (2 * N * ( DotProduct[ I,N] ))
    const float factor = 2.0f * ((vector.X * normal.X) + (vector.Y * normal.Y) + (vector.Z * normal.Z));

    return Vector3(vector.X - (normal.X * factor), vector.Y - (normal.Y * factor), vector.Z - (normal.Z * factor));
  }


  void Vector3::Reflect(Vector3& rResult, const Vector3& vector, const Vector3& normal)
  {
    // I is the original array
    // N is the normal of the incident plane
    // R = I - (2 * N * ( DotProduct[ I,N] ))
    const float factor = 2.0f * ((vector.X * normal.X) + (vector.Y * normal.Y) + (vector.Z * normal.Z));

    rResult.X = vector.X - (normal.X * factor);
    rResult.Y = vector.Y - (normal.Y * factor);
    rResult.Z = vector.Z - (normal.Z * factor);
  }


  Vector3 Vector3::SmoothStep(const Vector3& value1, const Vector3& value2, const float amount)
  {
    return Vector3(MathHelper::SmoothStep(value1.X, value2.X, amount), MathHelper::SmoothStep(value1.Y, value2.Y, amount),
                   MathHelper::SmoothStep(value1.Z, value2.Z, amount));
  }


  void Vector3::SmoothStep(const Vector3& value1, const Vector3& value2, const float amount, Vector3& rResult)
  {
    rResult.X = MathHelper::SmoothStep(value1.X, value2.X, amount);
    rResult.Y = MathHelper::SmoothStep(value1.Y, value2.Y, amount);
    rResult.Z = MathHelper::SmoothStep(value1.Z, value2.Z, amount);
  }

  Vector3 Vector3::Transform(const Vector3& position, const Matrix& matrix)
  {
    Vector3 result(OptimizationFlag::NoInitialization);
    MatrixInternals::Transform(result, position, matrix);
    return result;
  }


  void Vector3::Transform(Vector3& rResult, const Vector3& position, const Matrix& matrix)
  {
    MatrixInternals::Transform(rResult, position, matrix);
  }


  Vector3 Vector3::Transform(const Vector3& vec, const Quaternion& quat)
  {
    Vector3 result;
    Transform(vec, quat, result);
    return result;
  }

  void Vector3::Transform(const Vector3& value, const Quaternion& rotation, Vector3& rResult)
  {
    const float x = 2 * (rotation.Y * value.Z - rotation.Z * value.Y);
    const float y = 2 * (rotation.Z * value.X - rotation.X * value.Z);
    const float z = 2 * (rotation.X * value.Y - rotation.Y * value.X);

    rResult.X = value.X + x * rotation.W + (rotation.Y * z - rotation.Z * y);
    rResult.Y = value.Y + y * rotation.W + (rotation.Z * x - rotation.X * z);
    rResult.Z = value.Z + z * rotation.W + (rotation.X * y - rotation.Y * x);
  }

  Vector3 Vector3::TransformNormal(const Vector3& position, const Matrix& matrix)
  {
    Vector3 result(OptimizationFlag::NoInitialization);
    MatrixInternals::TransformNormal(result, position, matrix);
    return result;
  }


  void Vector3::TransformNormal(Vector3& rResult, const Vector3& position, const Matrix& matrix)
  {
    MatrixInternals::TransformNormal(rResult, position, matrix);
  }


}
