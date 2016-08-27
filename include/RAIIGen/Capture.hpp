#ifndef MB_CAPTURE_HPP
#define MB_CAPTURE_HPP
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

#include <RAIIGen/CaptureConfig.hpp>
#include <RAIIGen/EnumRecord.hpp>
#include <RAIIGen/FunctionRecord.hpp>
#include <RAIIGen/FunctionErrors.hpp>
#include <RAIIGen/StructRecord.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <clang-c/Index.h>


namespace MB
{
  class Capture
  {
    enum class CaptureMode
    {
      Off,
      Struct,
      Enum
    };

    struct CaptureInfo
    {
      CaptureMode Mode;
      std::size_t Level;
      CaptureInfo()
        : Mode(CaptureMode::Off)
        , Level(0)
      {
      }

      CaptureInfo(const CaptureMode mode, const std::size_t level)
        : Mode(mode)
        , Level(level)
      {
      }
    };

    CaptureConfig m_config;
    std::size_t m_level;
    std::deque<FunctionRecord> m_records;
    std::deque<FunctionErrors> m_functionErrors;
    std::unordered_map<std::string, StructRecord> m_structs;
    std::unordered_map<std::string, EnumRecord> m_enums;
    std::deque<CaptureInfo> m_captureInfo;
    std::deque<StructRecord> m_captureStructs;
    std::deque<EnumRecord> m_captureEnums;
  public:
    Capture(const CaptureConfig& config, CXCursor rootCursor);
    CXChildVisitResult OnVisit(CXCursor cursor, CXCursor parent);

    void Dump();

    const std::unordered_map<std::string, StructRecord>& GetStructDict() const
    {
      return m_structs;
    }

    const std::unordered_map<std::string, EnumRecord>& GetEnumDict() const
    {
      return m_enums;
    }

    const std::deque<FunctionRecord>& GetFunctions() const
    {
      return m_records;
    }

    std::deque<FunctionRecord>& DirectFunctions()
    {
      return m_records;
    }

    static CXChildVisitResult VistorForwarder(CXCursor cursor, CXCursor parent, CXClientData clientData);
  private:
  };
}
#endif
