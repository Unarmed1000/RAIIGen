#ifndef FSLBASE_MATH_RECTANGLE9_HPP
#define FSLBASE_MATH_RECTANGLE9_HPP
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

#include <FslBase/Math/Rectangle.hpp>

namespace Fsl
{
  struct Rectangle9
  {
    Rectangle Array[9];

    Rectangle9();
    Rectangle9(const Rectangle& topLeft, const Rectangle& topCenter, const Rectangle& topRight, const Rectangle& Left, const Rectangle& Center,
               const Rectangle& Right, const Rectangle& bottomLeft, const Rectangle& bottomCenter, const Rectangle& bottomRight);

    Rectangle TopLeft() const
    {
      return Array[0];
    }
    Rectangle TopCenter() const
    {
      return Array[1];
    }
    Rectangle TopRight() const
    {
      return Array[2];
    }
    Rectangle Left() const
    {
      return Array[3];
    }
    Rectangle Center() const
    {
      return Array[4];
    }
    Rectangle Right() const
    {
      return Array[5];
    }
    Rectangle BottomLeft() const
    {
      return Array[6];
    }
    Rectangle BottomCenter() const
    {
      return Array[7];
    }
    Rectangle BottomRight() const
    {
      return Array[8];
    }

    void SetTopLeft(const Rectangle& value)
    {
      Array[0] = value;
    }
    void SetTopCenter(const Rectangle& value)
    {
      Array[1] = value;
    }
    void SetTopRight(const Rectangle& value)
    {
      Array[2] = value;
    }
    void SetLeft(const Rectangle& value)
    {
      Array[3] = value;
    }
    void SetCenter(const Rectangle& value)
    {
      Array[4] = value;
    }
    void SetRight(const Rectangle& value)
    {
      Array[5] = value;
    }
    void SetBottomLeft(const Rectangle& value)
    {
      Array[6] = value;
    }
    void SetBottomCenter(const Rectangle& value)
    {
      Array[7] = value;
    }
    void SetBottomRight(const Rectangle& value)
    {
      Array[8] = value;
    }

    bool operator==(const Rectangle9& rhs) const
    {
      return (Array[0] == rhs.Array[0] && Array[1] == rhs.Array[1] && Array[2] == rhs.Array[2] && Array[3] == rhs.Array[3] &&
              Array[4] == rhs.Array[4] && Array[5] == rhs.Array[5] && Array[6] == rhs.Array[6] && Array[7] == rhs.Array[7] &&
              Array[8] == rhs.Array[8]);
    }


    bool operator!=(const Rectangle9& rhs) const
    {
      return !(*this == rhs);
    }
  };
}


#endif
