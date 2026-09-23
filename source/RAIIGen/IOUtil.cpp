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

#include <RAIIGen/IOUtil.hpp>
#include <FslBase/IO/File.hpp>
#include <filesystem>
#include <memory>

namespace MB
{
  using namespace Fsl;

  void IOUtil::WriteAllTextIfChanged(const IO::Path& fileName, const std::string& content)
  {
    std::string currentContent;
    // Skip the write if the file already exist and if it contains the exact same content.
    if (IO::File::TryReadAllText(currentContent, fileName) && currentContent == content)
      return;
    IO::File::WriteAllText(fileName, content);
  }


  bool IOUtil::TryGetDirectories(IO::PathDeque& rResult, const IO::Path& path, const IO::SearchOptions searchOptions)
  {
    const auto toPath = [](const std::filesystem::path& entryPath)
    {
      const auto str = entryPath.generic_u8string();
      return std::make_shared<IO::Path>(std::string(str.begin(), str.end()));
    };

    const auto& utf8Path = path.ToUTF8String();
    const std::filesystem::path srcPath(std::u8string(utf8Path.begin(), utf8Path.end()));
    std::error_code error;
    if (searchOptions == IO::SearchOptions::AllDirectories)
    {
      for (std::filesystem::recursive_directory_iterator itr(srcPath, error), end; !error && itr != end; itr.increment(error))
      {
        if (itr->is_directory())
          rResult.push_back(toPath(itr->path()));
      }
    }
    else
    {
      for (std::filesystem::directory_iterator itr(srcPath, error), end; !error && itr != end; itr.increment(error))
      {
        if (itr->is_directory())
          rResult.push_back(toPath(itr->path()));
      }
    }
    return !error;
  }
}
