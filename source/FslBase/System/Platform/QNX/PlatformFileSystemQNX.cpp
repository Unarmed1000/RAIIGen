#ifdef __QNXNTO__
/****************************************************************************************************************************************************
* Copyright (c) 2015 Freescale Semiconductor, Inc.
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

#include <FslBase/System/Platform/PlatformFileSystem.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/PathDeque.hpp>
#include <FslBase/Log/Log.hpp>
#include <sys/types.h>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

namespace Fsl
{
  namespace IO
  {

    struct FileData
    {
      bool IsDataValid;
      struct stat AttributeData;

      FileData()
        : IsDataValid(false)
      {
      }

      bool operator==(const FileData& rhs) const
      {
        if (!IsDataValid)
          return !rhs.IsDataValid;

        return AttributeData.st_ctime == rhs.AttributeData.st_ctime;
      }

      bool operator!=(const FileData& rhs) const
      {
        return !(*this == rhs);
      }
    };

    class PlatformPathMonitorToken
    {
    public:
      Path FullPath;
      FileData Data;

      PlatformPathMonitorToken(const Path& fullPath)
        : FullPath(fullPath)
        , Data()
      {
      }
    };


    namespace
    {
      void ExtractData(FileData& rData, const Path& fullPath)
      {
        rData = FileData();
        rData.IsDataValid = (stat(fullPath.ToUTF8String().c_str(), &rData.AttributeData) == 0);
      }


      void GetFilesInDirectory(PathDeque& rResult, const IO::Path& path, const bool includeSubdirectories)
      {
        DIR* pDir = opendir(path.ToUTF8String().c_str());
        if (pDir != nullptr)
        {
          struct dirent* pEnt;
          // print all the files and directories within directory
          while ((pEnt = readdir(pDir)) != nullptr)
          {
            if (std::strcmp(pEnt->d_name, ".") != 0 && std::strcmp(pEnt->d_name, "..") != 0)
            {
              const Path fullPath(Path::Combine(path, pEnt->d_name));
              FileAttributes attr;
              if (PlatformFileSystem::TryGetAttributes(fullPath, attr))
              {
                if ( attr.HasFlag(FileAttributes::File))
                  rResult.push_back(std::shared_ptr<Path>(new Path(fullPath)));
                else if (includeSubdirectories && attr.HasFlag(FileAttributes::Directory))
                  GetFilesInDirectory(rResult, fullPath, true);
              }
            }
          }
          closedir(pDir);
        }
        else
          throw DirectoryNotFoundException(path.ToAsciiString());

      } //GetFilesInDirectory()

    } //namespace


    bool PlatformFileSystem::TryGetAttributes(const Path& path, FileAttributes& rAttributes)
    {
      rAttributes = FileAttributes();

      struct stat s;
      if( stat(path.ToUTF8String().c_str(),&s) != 0 )
        return false;

      if( S_ISDIR(s.st_mode) )
        rAttributes.SetFlag(FileAttributes::Directory);
      else if( S_ISREG(s.st_mode) )
        rAttributes.SetFlag(FileAttributes::File);
      return true;
    }


    std::shared_ptr<PlatformPathMonitorToken> PlatformFileSystem::CreatePathMonitorToken(const Path& fullPath)
    {
      if (!Path::IsPathRooted(fullPath))
        throw std::invalid_argument("path must be rooted");

      std::shared_ptr<PlatformPathMonitorToken> result(new PlatformPathMonitorToken(fullPath));
      ExtractData(result->Data, fullPath);
      return result;
    }


    bool PlatformFileSystem::CheckPathForChanges(const std::shared_ptr<PlatformPathMonitorToken>& token)
    {
      if (!token)
        throw std::invalid_argument("token can not be null");

      FileData data;
      ExtractData(data, token->FullPath);

      if (data != token->Data)
      {
        token->Data = data;
        return true;
      }
      return false;
    }


    void PlatformFileSystem::GetFiles(PathDeque& rResult, const Path& path, const SearchOptions::Enum searchOptions)
    {
      rResult.clear();
      FileAttributes attr;
      if (! TryGetAttributes(path, attr) || ! attr.HasFlag(FileAttributes::Directory))
        throw DirectoryNotFoundException(path.ToAsciiString());

      switch (searchOptions)
      {
      case SearchOptions::TopDirectoryOnly:
        GetFilesInDirectory(rResult, path, false);
        break;
      case SearchOptions::AllDirectories:
        GetFilesInDirectory(rResult, path, true);
        break;
      default:
        throw NotSupportedException("Unknown search option");
      }
    }
  }
}

#endif
