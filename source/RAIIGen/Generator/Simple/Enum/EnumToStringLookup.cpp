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

#include <RAIIGen/Generator/Simple/Enum/EnumToStringLookup.hpp>
#include <RAIIGen/Generator/Simple/Enum/EnumToStringSnippets.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/IOUtil.hpp>
#include <RAIIGen/Generator/ConfigUtil.hpp>
#include <RAIIGen/StringHelper.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/Directory.hpp>
#include <FslBase/IO/File.hpp>
#include <FslBase/IO/Path.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <algorithm>
#include <iostream>
#include <fmt/format.h>

namespace MB
{
  using namespace Fsl;

  namespace
  {
    std::string GetRelativeIncludeGuard(const IO::Path& rootPath, const IO::Path& dstFilename)
    {
      auto filename = dstFilename.ToUTF8String();
      auto root = rootPath.ToUTF8String();

      if (!StringUtil::EndsWith(root, "/"))
        root += "/";

      if (!StringUtil::StartsWith(filename, root))
        throw UsageErrorException("The dstFile name was not based on the given root path");

      if (root.size() > 0)
        filename.erase(0, root.size());

      StringUtil::Replace(filename, "/", "_");
      StringUtil::Replace(filename, ".", "_");
      return CaseUtil::UpperCase(filename);
    }

    EnumToStringSnippets LoadSnippets(const IO::Path& templateRoot)
    {
      EnumToStringSnippets snippets;
      const auto pathHeader = IO::Path::Combine(templateRoot, "enumDebugStrings/Template_header.hpp");
      const auto pathMethod = IO::Path::Combine(templateRoot, "enumDebugStrings/Template_method.txt");
      const auto pathCaseEntry = IO::Path::Combine(templateRoot, "enumDebugStrings/Template_case_entry.txt");

      snippets.Header = IO::File::ReadAllText(pathHeader);
      snippets.Method = IO::File::ReadAllText(pathMethod);
      snippets.CaseEntry = IO::File::ReadAllText(pathCaseEntry);
      return snippets;
    }

    std::string GenerateEnumMember(const VersionGuardConfig& versionGuard, const std::string& snippet, const EnumMemberRecord& enumMember)
    {
      //guardConfig.ToGuardString(version);
      std::string content(snippet);
      StringUtil::Replace(content, "##ENUM_MEMBER_NAME##", enumMember.Name);

      if (versionGuard.IsValid && enumMember.Version != VersionRecord())
      {
        content = fmt::format("#if {0}{1}{2}{1}#endif", versionGuard.ToGuardString(enumMember.Version), END_OF_LINE, content);
      }
      return content;
    }


    std::string GenerateHeaderFile(const SimpleGeneratorConfig& config, const std::string& snippetHeader, const IO::Path& dstRootPath, 
                                   const IO::Path& dstFileName, const std::string& content, const VersionRecord& version = VersionRecord())
    {
      std::string strVersinGuardBegin;
      std::string strVersinGuardEnd;


      if (config.VersionGuard.IsValid && version != VersionRecord())
      {
        strVersinGuardBegin = fmt::format(END_OF_LINE + "#if {0}", config.VersionGuard.ToGuardString(version));
        strVersinGuardEnd = END_OF_LINE + "#endif";
      }

      std::string headerContent = snippetHeader;
      StringUtil::Replace(headerContent, "##METHODS##", content);
      StringUtil::Replace(headerContent, "##NAMESPACE_NAME##", config.NamespaceName);
      StringUtil::Replace(headerContent, "##NAMESPACE_NAME!##", CaseUtil::UpperCase(config.NamespaceName));
      StringUtil::Replace(headerContent, "##AG_TOOL_STATEMENT##", config.ToolStatement);
      StringUtil::Replace(headerContent, "##RELATIVE_INCLUDE_GUARD##", GetRelativeIncludeGuard(dstRootPath, dstFileName));

      StringUtil::Replace(headerContent, "##VERSION_GUARD_BEGIN##", strVersinGuardBegin);
      StringUtil::Replace(headerContent, "##VERSION_GUARD_END##", strVersinGuardEnd);
      return headerContent;
    }


    void ProcessOneFile(const EnumToStringSnippets& snippets, const Capture& capture, const SimpleGeneratorConfig& config, 
                        const IO::Path& dstRootPath, const IO::Path& dstFileName, const bool useSeperateFiles)
    {
      std::string content;

      auto enumDict = capture.GetEnumDict();
      bool isFirst = true;

      const std::string endOfLine3 = END_OF_LINE + END_OF_LINE + END_OF_LINE;
     
      std::vector<std::string> sortedNames(enumDict.size());
      {
        std::size_t index = 0;
        for (const auto& entry : enumDict)
        {
          sortedNames[index] = entry.first;
          ++index;
        }
        std::sort(sortedNames.begin(), sortedNames.end());
      }


      for (const auto& entryName : sortedNames)
      {
        std::size_t caseCount = 0;
        ConfigUtil::CurrentEntityInfo currentEnumEntityInfo(entryName);
        if (!ConfigUtil::HasMatchingEntry(entryName, config.EnumNameBlacklist, currentEnumEntityInfo))
        {
          const auto itrFind = enumDict.find(entryName);
          std::string switchCaseContent;
          for (const auto& enumMember : itrFind->second.Members)
          {
            if (!ConfigUtil::HasMatchingEntry(enumMember.Name, config.EnumMemberBlacklist, currentEnumEntityInfo))
            {
              switchCaseContent += END_OF_LINE + GenerateEnumMember(config.VersionGuard, snippets.CaseEntry, enumMember);
              ++caseCount;
            }
          }
          if (caseCount > 0)
          {
            std::string methodContent = snippets.Method;
            if (config.VersionGuard.IsValid && itrFind->second.Version != VersionRecord())
            {
              methodContent = fmt::format("#if {0}{1}{2}{1}#endif", config.VersionGuard.ToGuardString(itrFind->second.Version), END_OF_LINE, methodContent);
            }

            StringUtil::Replace(methodContent, "##C_TYPE_NAME##", itrFind->second.Name);
            StringUtil::Replace(methodContent, "##CASE_ENTRIES##", switchCaseContent);
            content += (!isFirst ? endOfLine3 : END_OF_LINE) + methodContent;

 
            isFirst = false;
          }
        }
      }

      std::string headerContent = GenerateHeaderFile(config, snippets.Header, dstRootPath, dstFileName, content);

      auto dirName = IO::Path::GetDirectoryName(dstFileName);
      IO::Directory::CreateDirectory(dirName);

      IOUtil::WriteAllTextIfChanged(dstFileName, headerContent);
    }


    void ProcessMultipleFile(const EnumToStringSnippets& snippets, const Capture& capture, const SimpleGeneratorConfig& config, const IO::Path& dstRootPath, const IO::Path& dstFilePath, const bool useSeperateFiles)
    {
      IO::Directory::CreateDirectory(dstFilePath);

      auto enumDict = capture.GetEnumDict();

      for (const auto& entry : enumDict)
      {
        std::size_t caseCount = 0;
        ConfigUtil::CurrentEntityInfo currentEnumEntityInfo(entry.first);
        if (!ConfigUtil::HasMatchingEntry(entry.first, config.EnumNameBlacklist, currentEnumEntityInfo))
        {
          std::string switchCaseContent;
          for (const auto& enumMember : entry.second.Members)
          {
            if (!ConfigUtil::HasMatchingEntry(enumMember.Name, config.EnumMemberBlacklist, currentEnumEntityInfo))
            {
              switchCaseContent += END_OF_LINE + GenerateEnumMember(config.VersionGuard, snippets.CaseEntry, enumMember);
              ++caseCount;
            }
          }
          if (caseCount > 0)
          {
            std::string methodContent = END_OF_LINE + snippets.Method;
            StringUtil::Replace(methodContent, "##C_TYPE_NAME##", entry.second.Name);
            StringUtil::Replace(methodContent, "##CASE_ENTRIES##", switchCaseContent);

            auto dstFileName = IO::Path::Combine(dstFilePath, entry.first + ".hpp");
            std::string headerContent = GenerateHeaderFile(config, snippets.Header, dstRootPath, dstFileName, methodContent, entry.second.Version);

            IOUtil::WriteAllTextIfChanged(dstFileName, headerContent);
          }
        }
      }
    }

  }



  void EnumToStringLookup::Process(const Capture& capture, const SimpleGeneratorConfig& config, const IO::Path& templateRoot, const IO::Path& dstRootPath, const IO::Path& dstFileName, const bool useSeperateFiles)
  {
    EnumToStringSnippets snippets = LoadSnippets(templateRoot);

    if (!useSeperateFiles)
      ProcessOneFile(snippets, capture, config, dstRootPath, dstFileName, useSeperateFiles);
    else
      ProcessMultipleFile(snippets, capture, config, dstRootPath, dstFileName, useSeperateFiles);
  }

}
