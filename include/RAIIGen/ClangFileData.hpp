#ifndef MB_CLANGFILEDATA_HPP
#define MB_CLANGFILEDATA_HPP
//***************************************************************************************************************************************************
//* BSD 3-Clause License
//*
//* Copyright (c) 2017, Rene Thrane
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

#include <RAIIGen/Generator/BasicConfig.hpp>
#include <FslBase/IO/Path.hpp>
#include <clang-c/Index.h>
#include <deque>
#include <vector>

namespace MB
{
  class ClangFileData
  {
    CXIndex m_index;
    CXTranslationUnit m_tu;
    CXCursor m_rootCursor;

  public:
    Fsl::IO::Path Filename;
    std::vector<Fsl::IO::Path> IncludePaths;

	ClangFileData(const BasicConfig& basicConfig, const Fsl::IO::Path& filename, const std::vector<Fsl::IO::Path>& includePaths)
      : Filename(filename)
      , IncludePaths(includePaths)
    {
      using namespace MB;

      // char* clangAargs[] =
      //{
      //  "-Ic:/Program Files (x86)/AMD APP SDK/3.0/include",
      //  "-Ie:/_sdk/amdovx-core/openvx/include",
      //  //"-I.",
      //  //"-I./include",
      //  //"-x",
      //  //"c++",
      //  //"-Xclang",
      //  //"-ast-dump",
      //  //"-fsyntax-only",
      //  //"-std=c++1y"
      //};

      std::deque<std::string> clangArgsTemp;
      for (std::size_t i = 0; i < includePaths.size(); ++i)
      {
        clangArgsTemp.push_back(std::string("-I") + includePaths[i].ToUTF8String());
      }
      clangArgsTemp.push_back("-DGL_GLEXT_PROTOTYPES");

      std::vector<const char*> clangArgs(clangArgsTemp.size());
      for (std::size_t i = 0; i < clangArgs.size(); ++i)
        clangArgs[i] = clangArgsTemp[i].c_str();

      m_index = clang_createIndex(0, 1);
      // m_tu = clang_createTranslationUnitFromSourceFile(m_index, filename.ToUTF8String().c_str(), static_cast<int>(clangArgs.size()),
      // clangArgs.data(), 0, nullptr);
      m_tu = clang_parseTranslationUnit(m_index, filename.ToUTF8String().c_str(), clangArgs.data(), static_cast<int>(clangArgs.size()), nullptr, 0,
                                        CXTranslationUnit_None);

      if (!m_tu)
      {
        std::cout << "Failed\n";
        throw std::runtime_error("Failed to translate source file");
      }

      m_rootCursor = clang_getTranslationUnitCursor(m_tu);

      //{
      //  unsigned int treeLevel = 0;
      //  //clang_visitChildren(m_rootCursor, ClangInfoDump::Visitor, &treeLevel);
      //}
    }

    ~ClangFileData()
    {
      clang_disposeTranslationUnit(m_tu);
      clang_disposeIndex(m_index);
    }

    CXIndex GetIndex() const
    {
      return m_index;
    }

    CXTranslationUnit GetTranslationUnit() const
    {
      return m_tu;
    }

    CXCursor GetRootCursor() const
    {
      return m_rootCursor;
    }
  };
}

#endif
