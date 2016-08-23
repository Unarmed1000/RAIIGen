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

#include <FslBase/System/Platform/ScopedSystemMemoryAccess.hpp>
#include <FslBase/System/Platform/SystemMemoryAccess.hpp>

namespace Fsl
{
  uint8_t SystemMemoryAccess::GetUInt8(const std::size_t targetAddress)
  {
    ScopedSystemMemoryAccess access(targetAddress);
    return access.GetUInt8();
  }


  uint16_t SystemMemoryAccess::GetUInt16(const std::size_t targetAddress)
  {
    ScopedSystemMemoryAccess access(targetAddress);
    return access.GetUInt16();
  }


  uint32_t SystemMemoryAccess::GetUInt32(const std::size_t targetAddress)
  {
    ScopedSystemMemoryAccess access(targetAddress);
    return access.GetUInt32();
  }


  void SystemMemoryAccess::SetUInt8(const std::size_t targetAddress, const uint8_t value)
  {
    ScopedSystemMemoryAccess access(targetAddress);
    return access.SetUInt8(value);
  }


  void SystemMemoryAccess::SetUInt16(const std::size_t targetAddress, const uint16_t value)
  {
    ScopedSystemMemoryAccess access(targetAddress);
    return access.SetUInt16(value);
  }


  void SystemMemoryAccess::SetUInt32(const std::size_t targetAddress, const uint32_t value)
  {
    ScopedSystemMemoryAccess access(targetAddress);
    return access.SetUInt32(value);
  }

}  
