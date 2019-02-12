#ifndef FSLBASE_GETOPT_OPTION_HPP
#define FSLBASE_GETOPT_OPTION_HPP
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

#include <FslBase/BasicTypes.hpp>
#include <FslBase/Getopt/OptionArgument.hpp>
#include <FslBase/Getopt/OptionGroup.hpp>
#include <FslBase/Getopt/OptionType.hpp>

namespace Fsl
{
  //! @brief Extension of the normal getopt option with a number of extra parameters
  struct Option
  {
    const char* ShortName;
    const char* Name;
    OptionArgument::Enum HasArg;
    int32_t CmdId;
    const char* Description;
    OptionGroup::Enum Group;
    OptionType::Enum Type;
    bool IsPositional;

    Option(const char* const pszSmartName, const OptionArgument::Enum hasArg, const int32_t cmdId, const char* const pszDescription);
    Option(const char* const pszSmartName, const OptionArgument::Enum hasArg, const int32_t cmdId, const char* const pszDescription,
           const OptionGroup::Enum group);
    Option(const char* const pszShortName, const char* const pszName, const OptionArgument::Enum hasArg, const int32_t cmdId,
           const char* const pszDescription);
    Option(const char* const pszShortName, const char* const pszName, const OptionArgument::Enum hasArg, const int32_t cmdId,
           const char* const pszDescription, const OptionGroup::Enum group);
  };


  struct PositionalOption : public Option
  {
    PositionalOption(const char* const pszName, const OptionArgument::Enum hasArg, const int32_t cmdId, const char* const pszDescription);
    PositionalOption(const char* const pszName, const OptionArgument::Enum hasArg, const int32_t cmdId, const char* const pszDescription,
                     const OptionGroup::Enum group);
  };
}

#endif
