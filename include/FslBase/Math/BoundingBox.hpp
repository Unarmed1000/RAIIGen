#ifndef FSLBASE_MATH_BOUNDINGBOX_HPP
#define FSLBASE_MATH_BOUNDINGBOX_HPP
/****************************************************************************************************************************************************
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
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

// The functions in this file are a port of an MIT licensed library: MonaGame - BoundingBox.cs.

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

#include <FslBase/BasicTypes.hpp>
#include <FslBase/Math/ContainmentType.hpp>
#include <FslBase/Math/PlaneIntersectionType.hpp>
#include <FslBase/Math/Vector3.hpp>
#include <vector>

namespace Fsl
{
  class BoundingFrustum;
  struct BoundingSphere;
  struct Plane;
  struct Ray;

  struct BoundingBox
  {
    static const int32_t CornerCount = 8;

    Vector3 Min;
    Vector3 Max;

    BoundingBox()
      : Min()
      , Max()
    {
    }

    BoundingBox(const Vector3& min, const Vector3& max)
      : Min(min)
      , Max(max)
    {
    }

    ContainmentType Contains(const BoundingBox& box) const;
    void Contains(const BoundingBox& box, ContainmentType& rResult) const;
    // ContainmentType Contains(const BoundingFrustum& frustum) const;
    ContainmentType Contains(const BoundingSphere& sphere) const;
    void Contains(const BoundingSphere& sphere, ContainmentType& rResult) const;
    ContainmentType Contains(const Vector3& point) const;
    void Contains(const Vector3& point, ContainmentType& rResult) const;

    //! @brief Create a bounding box from the given list of points.
    //! @param points The list of Vector3 instances defining the point cloud to bound
    //! @return A bounding box that encapsulates the given point cloud.
    /// @throws std::invalid_argument Thrown if the given vector has no points.
    static BoundingBox CreateFromPoints(const std::vector<Vector3>& points);
    static BoundingBox CreateFromSphere(const BoundingSphere& sphere);
    static void CreateFromSphere(const BoundingSphere& sphere, BoundingBox& rResult);
    static BoundingBox CreateMerged(const BoundingBox& original, const BoundingBox& additional);
    static void CreateMerged(const BoundingBox& original, const BoundingBox& additional, BoundingBox& rResult);

    std::vector<Vector3> GetCorners() const;
    void GetCorners(std::vector<Vector3>& corners) const;

    bool Intersects(const BoundingBox& box) const;
    void Intersects(const BoundingBox& box, bool& rResult) const;
    bool Intersects(const BoundingFrustum& frustum) const;
    bool Intersects(const BoundingSphere& sphere) const;
    void Intersects(const BoundingSphere& sphere, bool& rResult) const;
    PlaneIntersectionType Intersects(const Plane& plane) const;
    void Intersects(const Plane& plane, PlaneIntersectionType& rResult) const;
    bool Intersects(const Ray& ray, float& rResult) const;

    //! @brief Tests for equality.
    bool operator==(const BoundingBox& rhs) const
    {
      return Min == rhs.Min && Max == rhs.Max;
    }

    //! @brief Tests for inequality.
    bool operator!=(const BoundingBox& rhs) const
    {
      return Min != rhs.Min || Max != rhs.Max;
    }
  };

}

#endif
