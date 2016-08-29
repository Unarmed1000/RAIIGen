#ifndef MB_GENERATOR_SIMPLE_MEMBERVARIABLE_HPP
#define MB_GENERATOR_SIMPLE_MEMBERVARIABLE_HPP
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

#include <RAIIGen/TypeRecord.hpp>
#include <string>

namespace MB
{
  struct MemberVariable
  {
    TypeRecord FullType;
    std::string Type;
    std::string Name;

    std::string ArgumentName;
    // The name of the original source argument
    std::string SourceArgumentName;
    std::string NiceNameUpperCamelCase;

    MemberVariable()
    {
    }

    MemberVariable(const TypeRecord& fullType, const std::string& type, const std::string& name, const std::string& argumentName, const std::string& niceNameUpperCamelCase)
      : FullType(fullType)
      , Type(type)
      , Name(name)
      , ArgumentName(argumentName)
      , SourceArgumentName(argumentName)
      , NiceNameUpperCamelCase(niceNameUpperCamelCase)
    {
    }

    bool IsValid() const
    {
      return Name.size() > 0;
    }

    bool operator==(const MemberVariable &rhs) const
    {
      return FullType == rhs.FullType &&
        Type == rhs.Type &&
        Name == rhs.Name &&
        ArgumentName == rhs.ArgumentName &&
        SourceArgumentName == rhs.SourceArgumentName &&
        NiceNameUpperCamelCase == rhs.NiceNameUpperCamelCase;
    }

    bool operator!=(const MemberVariable &rhs) const
    {
      return !(*this == rhs);
    }
  };
}
#endif
