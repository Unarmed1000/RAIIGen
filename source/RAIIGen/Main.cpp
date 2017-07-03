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

#include <clang-c/Index.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <FslBase/IO/Directory.hpp>
#include <FslBase/IO/Path.hpp>
#include <FslBase/IO/PathDeque.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <RAIIGen/ProgramInfo.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/CapturedData.hpp>
#include <RAIIGen/CustomLogConsole.hpp>
#include <RAIIGen/Generator/BasicConfig.hpp>
#include <RAIIGen/Generator/OpenCLGenerator.hpp>
#include <RAIIGen/Generator/OpenGLESGenerator.hpp>
#include <RAIIGen/Generator/OpenVXGenerator.hpp>
#include <RAIIGen/Generator/VulkanGenerator.hpp>
#include <RAIIGen/ClangInfoDump.hpp>

namespace MB
{
  namespace
  {
    const std::string g_programName = "RAIIGen";
    const std::string g_programVersion = "V0.4.0";

    using namespace Fsl;

    struct Config
    {
      IO::Path HeaderRoot;
      IO::Path TemplateRoot;
      IO::Path OutputRoot;

      Config()
      {
      }


      Config(const IO::Path& headerRoot, const IO::Path& templateRoot, const IO::Path& outputRoot)
        : HeaderRoot(headerRoot)
        , TemplateRoot(templateRoot)
        , OutputRoot(outputRoot)
      {
      }
    };


    std::deque<std::shared_ptr<CapturedData> > RunCaptureHistory(const BasicConfig& basicConfig, const IO::Path& relativeFilename, const IO::Path& historyPath, const MB::CaptureConfig& captureConfig)
    {
      using namespace MB;
      const bool useMajorVersion = true;
      std::string sourceAPIVersion = basicConfig.APIVersion;
      if(useMajorVersion)
      {
        const auto dotIndex = StringUtil::IndexOf(sourceAPIVersion, '.');
        if (dotIndex >= 0)
          sourceAPIVersion = sourceAPIVersion.substr(0, dotIndex);
      }


      const std::string apiVersion = sourceAPIVersion + ".";

      std::deque<std::shared_ptr<CapturedData> > history;

      // Scan for history files
      IO::PathDeque entries;
      if (!IO::Directory::TryGetDirectories(entries, historyPath, IO::SearchOptions::TopDirectoryOnly))
        return history;

      // For now we disable the logger during history parsing
      const std::shared_ptr<CustomLog> customLog;

      std::cout << "Scanning API history" << "\n";
      for (const auto& entry : entries)
      {
        const auto dirName = IO::Path::GetFileName(*entry);
        if (dirName.StartsWith(apiVersion))
        {
          std::cout << "- '" << dirName.ToUTF8String() << "'\n";

          const auto srcFile = IO::Path::Combine(*entry, relativeFilename);

          VersionRecord version(dirName.ToUTF8String());

          // Create the history
          const std::vector<IO::Path> includePaths = { *entry };
          history.push_back(std::make_shared<CapturedData>(basicConfig, srcFile, includePaths, captureConfig, customLog, version));
        }
      }
      
      auto sortMethod = [](const std::shared_ptr<CapturedData> &lhs, const std::shared_ptr<CapturedData> &rhs)
      { 
        const uint64_t domain = 1000000;
        assert(lhs->Version.Major < domain);
        assert(lhs->Version.Minor < domain);
        assert(lhs->Version.Build < domain);
        assert(rhs->Version.Major < domain);
        assert(rhs->Version.Minor < domain);
        assert(rhs->Version.Build < domain);
        uint64_t lhsVal = static_cast<uint64_t>(lhs->Version.Major * 2 * domain) + static_cast<uint64_t>(lhs->Version.Minor * domain) + static_cast<uint64_t>(lhs->Version.Build);
        uint64_t rhsVal = static_cast<uint64_t>(rhs->Version.Major * 2 * domain) + static_cast<uint64_t>(rhs->Version.Minor * domain) + static_cast<uint64_t>(rhs->Version.Build);
        return lhsVal < rhsVal;
      };

      std::sort(history.begin(), history.end(), sortMethod);
      return history;
    }


    std::shared_ptr<CapturedData> LocateFirstAppearanceOfStruct(const std::deque<std::shared_ptr<CapturedData> >& history, const std::string& name)
    {
      for (const auto& capture : history)
      {
        const auto structDict = capture->TheCapture.DirectAccessStructDict();
        const auto itrFind = structDict.find(name);
        if (itrFind != structDict.end())
          return capture;
      }

      // This means that the file we are scanning is not part of the history, so its a user error
      throw std::runtime_error("Could not be found in history");
    }


    std::shared_ptr<CapturedData> LocateFirstAppearanceOfFunction(const std::deque<std::shared_ptr<CapturedData> >& history, const std::string& name)
    {
      for (const auto& capture : history)
      {
        const auto functions = capture->TheCapture.GetFunctions();
        const auto itrFind = std::find_if(functions.begin(), functions.end(), [name](const FunctionRecord& val) { return val.Name == name; });
        if (itrFind != functions.end())
          return capture;
      }

      // This means that the file we are scanning is not part of the history, so its a user error
      throw std::runtime_error("Could not be found in history");
    }


    std::shared_ptr<CapturedData> LocateFirstAppearanceOfEnum(const std::deque<std::shared_ptr<CapturedData> >& history, const std::string& name)
    {
      for (const auto& capture : history)
      {
        const auto enumDict = capture->TheCapture.GetEnumDict();
        
        const auto itrFind = enumDict.find(name);
        if (itrFind != enumDict.end())
          return capture;
      }

      // This means that the file we are scanning is not part of the history, so its a user error
      throw std::runtime_error("Could not be found in history");
    }

    std::shared_ptr<CapturedData>  LocateFirstAppearanceOfEnumMember(const std::deque<std::shared_ptr<CapturedData> >& history, const std::string& enumName, const std::string& enumMemberName)
    {
      for (const auto& capture : history)
      {
        const auto enumDict = capture->TheCapture.GetEnumDict();

        const auto itrFind = enumDict.find(enumName);
        if (itrFind != enumDict.end())
        {
          const auto& members = itrFind->second.Members;
          const auto itrFindMember = std::find_if(members.begin(), members.end(), [enumMemberName](const EnumMemberRecord& val) { return val.Name == enumMemberName; });
          if (itrFindMember != members.end())
            return capture;
        }
      }

      // This means that the file we are scanning is not part of the history, so its a user error
      throw std::runtime_error("Could not be found in history");
    }



    void TagStructsWithHistory(CapturedData& rCapturedData, const std::deque<std::shared_ptr<CapturedData> >& history)
    {
      for (auto& rEntry : rCapturedData.TheCapture.DirectAccessStructDict())
      {
        const auto historyEntry = LocateFirstAppearanceOfStruct(history, rEntry.second.Name);
        rEntry.second.Version = historyEntry->Version;
      }

      for (auto& rEntry : rCapturedData.TheCapture.DirectAccessStructs())
      {
        const auto historyEntry = LocateFirstAppearanceOfStruct(history, rEntry.Name);
        rEntry.Version = historyEntry->Version;
      }
    }


    void TagFunctionsWithHistory(CapturedData& rCapturedData, const std::deque<std::shared_ptr<CapturedData> >& history)
    {
      for (auto& rEntry : rCapturedData.TheCapture.DirectAccessFunctions())
      {
        const auto historyEntry = LocateFirstAppearanceOfFunction(history, rEntry.Name);
        rEntry.Version = historyEntry->Version;
      }
    }


    void TagEnumsWithHistory(CapturedData& rCapturedData, const std::deque<std::shared_ptr<CapturedData> >& history)
    {
      for (auto& rEntry : rCapturedData.TheCapture.DirectAccessEnumDict())
      {
        const auto historyEntry = LocateFirstAppearanceOfEnum(history, rEntry.second.Name);
        // Tag the enum with the version it was introduced in
        rEntry.second.Version = historyEntry->Version;

        for (auto& rEnumMember : rEntry.second.Members)
        {
          // Tag each individual enum member with the version it was introduced in.
          const auto memberHistoryEntry = LocateFirstAppearanceOfEnumMember(history, rEntry.second.Name, rEnumMember.Name);
          rEnumMember.Version = memberHistoryEntry->Version;
        }
      }
    }


    void TagWithHistory(CapturedData& rCapturedData, const std::deque<std::shared_ptr<CapturedData> >& history)
    {
      std::cout << "- Enums\n";
      TagEnumsWithHistory(rCapturedData, history);
      std::cout << "- Functions\n";
      TagFunctionsWithHistory(rCapturedData, history);
      std::cout << "- Structs\n";
      TagStructsWithHistory(rCapturedData, history);
    }


    template<typename TGenerator>
    void Run(const BasicConfig& basicConfig, const IO::Path& filename, const IO::Path& relativeFilename, const IO::Path& templatePath, const IO::Path& apiHistoryPath, const IO::Path& dstPath, const std::vector<IO::Path>& includePaths, const bool useAPIHistory)
    {
      using namespace MB;

      // Ensure that the dst path exist
      IO::Directory::CreateDirectory(dstPath);

      const std::shared_ptr<CustomLog> customLog = std::make_shared<CustomLogConsole>();

      const auto captureConfig = TGenerator::GetCaptureConfig();

      VersionRecord version("0.0.0");
      CapturedData capturedData(basicConfig, filename, includePaths, captureConfig, customLog, version);

      if (useAPIHistory)
      {
        auto history = RunCaptureHistory(basicConfig, relativeFilename, apiHistoryPath, captureConfig);
        if (history.size() > 0)
        {
          std::cout << "Version tagging elements using history\n";
          // Always use 0.0.0 for the first version
          history.front()->Version = VersionRecord();
          TagWithHistory(capturedData, history);
        }
       
      }

      //captureConfig.GetCapture().Dump();
      TGenerator generator(capturedData.TheCapture, basicConfig, templatePath, dstPath);
    }


    template<typename TGenerator>
    void RunGenerator(const ProgramInfo& programInfo, const Config& config, const std::string& filename, const std::string& templateName, const std::string& baseApiName, const std::string& apiVersion, const bool useAPIHistory = false)
    {
      const auto apiNameAndVersion = baseApiName + apiVersion;
      const auto templateNameAndVersion = templateName + apiVersion;
      const auto apiHeaderPath = IO::Path::Combine(config.HeaderRoot, IO::Path::Combine("khronos", apiNameAndVersion));
      const auto templatePath = IO::Path::Combine(config.TemplateRoot, templateName);
      const auto srcFile = IO::Path::Combine(apiHeaderPath, filename);
      const auto dstPath = IO::Path::Combine(config.OutputRoot, templateNameAndVersion);
      const auto apiHistoryPath = IO::Path::Combine(apiHeaderPath, "history");

      const std::vector<IO::Path> includePaths = { apiHeaderPath };
      std::cout << "*** Running " << apiNameAndVersion << " generator ***\n";


      const auto toolStatement = std::string("Auto-generated ") + baseApiName + " " + apiVersion + " C++11 RAII classes by " + programInfo.Name + " (https://github.com/Unarmed1000/RAIIGen)";

      auto namespaceName = apiNameAndVersion;
      StringUtil::Replace(namespaceName, ".", "_");

      BasicConfig basicConfig(programInfo, toolStatement, namespaceName, baseApiName, apiVersion);

      Run<TGenerator>(basicConfig, srcFile, filename, templatePath, apiHistoryPath, dstPath, includePaths, useAPIHistory);
    } 


    void GenerateClasses(const ProgramInfo& programInfo, const IO::Path& currentWorkingDirectory)
    {
      const auto headerRoot = IO::Path::Combine(currentWorkingDirectory, "config/Headers");
      const auto templateRoot = IO::Path::Combine(currentWorkingDirectory, "config/Templates");
      const auto outputRoot = IO::Path::Combine(currentWorkingDirectory, "output");

      Config config(headerRoot, templateRoot, outputRoot);

      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "OpenCL", "OpenCL", "1.1");
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "OpenCL", "OpenCL", "1.2");
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "OpenCL", "OpenCL", "2.0");
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "OpenCL", "OpenCL", "2.1");
      //RunGenerator<MB::OpenVXGenerator>(programInfo, config, "VX/vx.h", "OpenVX", "OpenVX", "1.0.1");
      //RunGenerator<MB::OpenVXGenerator>(programInfo, config, "VX/vx.h", "OpenVX", "OpenVX", "1.1");
      //RunGenerator<MB::VulkanGenerator>(programInfo, config, "vulkan/vulkan.h", "Vulkan", "Vulkan", "1.0");

      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "RapidOpenCL", "OpenCL", "1.1");
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "RapidOpenCL", "OpenCL", "1.2");
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "RapidOpenCL", "OpenCL", "2.0");
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "RapidOpenCL", "OpenCL", "2.1");
      //RunGenerator<MB::OpenVXGenerator>(programInfo, config, "VX/vx.h", "RapidOpenVX", "OpenVX", "1.0.1");
      RunGenerator<MB::OpenVXGenerator>(programInfo, config, "VX/vx.h", "RapidOpenVX", "OpenVX", "1.1", true);
      
      // RapidVulkan
      //RunGenerator<MB::VulkanGenerator>(programInfo, config, "vulkan/vulkan.h", "RapidVulkan", "Vulkan", "1.0", true);
      
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "FslUtilOpenCL", "OpenCL", "1.1");
      //RunGenerator<MB::OpenCLGenerator>(programInfo, config, "CL/cl.h", "FslUtilOpenCL", "OpenCL", "1.2");
      //RunGenerator<MB::OpenVXGenerator>(programInfo, config, "VX/vx.h", "FslUtilOpenVX", "OpenVX", "1.0.1");
      //RunGenerator<MB::OpenVXGenerator>(programInfo, config, "VX/vx.h", "FslUtilOpenVX", "OpenVX", "1.1");
      //RunGenerator<MB::VulkanGenerator>(programInfo, config, "vulkan/vulkan.h", "FslUtil.Vulkan", "Vulkan", "1.0");

      //RunGenerator<MB::OpenGLESGenerator>(programInfo, config, "GLES2/gl2.h", "RapidOpenGLES", "OpenGLES", "2.0");
      //RunGenerator<MB::OpenGLESGenerator>(programInfo, config, "GLES3/gl3.h", "RapidOpenGLES", "OpenGLES", "3.0");
      //RunGenerator<MB::OpenGLESGenerator>(programInfo, config, "GLES3/gl31.h", "RapidOpenGLES", "OpenGLES", "3.1");
      //RunGenerator<MB::OpenGLESGenerator>(programInfo, config, "GLES3/gl32.h", "RapidOpenGLES", "OpenGLES", "3.2");
    }
  }
}



int main(int argc, char** argv)
{
  using namespace MB;

  std::cout << g_programName << " " << g_programVersion << "\n";

  const auto currentPath = IO::Directory::GetCurrentWorkingDirectory();

  try
  {
    GenerateClasses(ProgramInfo(g_programName, g_programVersion), currentPath);
  }
  catch (const std::exception& ex)
  {
    std::cout << "ERROR: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}