#ifndef MB_STRINGHELPER_HPP
#define MB_STRINGHELPER_HPP
//***************************************************************************************************************************************************
//* BSD 3-Clause License
//*
//* Copyright (c) 2016, Rene Thrane
//* All rights reserved.
//*
//* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
//*    documentation and/or other materials provided with the distribution.
//* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this
//*    software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
//* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//***************************************************************************************************************************************************

#include <string>

namespace MB
{
  //! @brief Only works on ASCII chars.
  class StringHelper
  {
  public:
    static std::string EnforceLowerCamelCaseNameStyle(const std::string& value);
    static std::string EnforceUpperCamelCaseNameStyle(const std::string& value);

    // https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html
    // Any parameter that is a structure containing a sType member must have a value of sType which is a valid VkStructureType value matching
    // the type of the structure. As a general rule, the name of this value is obtained by taking the structure name, stripping the leading Vk,
    // prefixing each capital letter with _, converting the entire resulting string to upper case, and prefixing it with VK_STRUCTURE_TYPE_.
    // For example, structures of type VkImageCreateInfo must have a sType value of VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO.
    static std::string GenerateVulkanStructFlagName(const std::string structName);
  };

  // Nasty, but ok for now
  const std::string END_OF_LINE = "\r\n";
}
#endif
