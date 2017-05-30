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

#include <RAIIGen/Generator/OpenGLESGenerator.hpp>
#include <RAIIGen/Generator/FunctionNamePair.hpp>
#include <RAIIGen/Generator/MatchedFunctionPair.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/Capture.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/File.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>


using namespace Fsl;

namespace MB
{

  namespace
  {
    const auto CREATE_FUNCTION = "glGen";
    const auto DESTROY_FUNCTION = "glDelete";

    const auto TYPE_NAME_PREFIX = "GL";
    const auto FUNCTION_NAME_PREFIX = "gl";

    // gl_enum, not good
    const auto ERRORCODE_TYPE_NAME = "";

    const auto DEFAULT_VALUE = "VK_NULL_HANDLE";


    const std::vector<FunctionNamePair> g_functionPairs
    {
      FunctionNamePair(CREATE_FUNCTION, DESTROY_FUNCTION),
    };


    // Manual matches for methods that don't follow 'standard' patterns
    const std::vector<FunctionNamePair> g_manualFunctionMatches
    {
      // Pipelines are destroyed with vkDestroyPipeline
      //FunctionNamePair("vkCreateGraphicsPipelines", "vkDestroyPipeline"),
    };


    const std::vector<RAIIClassCustomization> g_arrayRAIIClassCustomization
    {
    };


    const std::vector<ClassFunctionAbsorb> g_classFunctionAbsorbtion
    {
    };


    const std::unordered_map<std::string, RAIIClassMethodOverrides> g_classMethodOverride =
    {
    };


    const std::vector<std::string> g_forceNullParameter
    {
      //"VkAllocationCallbacks"
    };


    const std::vector<FunctionGuard> g_functionGuards
    {
    };


    const std::vector<BlackListEntry> g_functionNameBlacklist
    {
    };


    const std::vector<BlackListEntry> g_functionNamePostfixBlacklist
    {
    };


    const std::vector<FunctionParameterNameOverride> g_functionParameterNameOverride
    {
    };


    const std::vector<FunctionParameterTypeOverride> g_functionParameterTypeOverride
    {
    };

    const std::unordered_map<std::string, std::string> g_typeDefaultValues =
    {
      //{ "VkBuffer", DEFAULT_VALUE },
      //{ "VkBufferView", DEFAULT_VALUE },
      //{ "VkCommandBuffer", DEFAULT_VALUE },
      //{ "VkCommandPool", DEFAULT_VALUE },
      //{ "VkDebugReportCallbackEXT", DEFAULT_VALUE },
      //{ "VkDescriptorPool", DEFAULT_VALUE },
      //{ "VkDescriptorSet", DEFAULT_VALUE },
      //{ "VkDescriptorSetLayout", DEFAULT_VALUE },
      //{ "VkDevice", DEFAULT_VALUE },
      //{ "VkDeviceMemory", DEFAULT_VALUE },
      //{ "VkEvent", DEFAULT_VALUE },
      //{ "VkFence", DEFAULT_VALUE },
      //{ "VkFramebuffer", DEFAULT_VALUE },
      //{ "VkImage", DEFAULT_VALUE },
      //{ "VkImageView", DEFAULT_VALUE },
      //{ "VkInstance", DEFAULT_VALUE },
      //{ "VkPipeline", DEFAULT_VALUE },
      //{ "VkPipelineCache", DEFAULT_VALUE },
      //{ "VkPipelineLayout", DEFAULT_VALUE },
      //{ "VkQueryPool", DEFAULT_VALUE },
      //{ "VkRenderPass", DEFAULT_VALUE },
      //{ "VkSampler", DEFAULT_VALUE },
      //{ "VkSemaphore", DEFAULT_VALUE },
      //{ "VkShaderModule", DEFAULT_VALUE },
      //{ "VkSurfaceKHR", DEFAULT_VALUE },
      //{ "VkSwapchainKHR", DEFAULT_VALUE },
    };
  }


  OpenGLESGenerator::OpenGLESGenerator(const Capture& capture, const BasicConfig& basicConfig, const Fsl::IO::Path& templateRoot, const Fsl::IO::Path& dstPath)
    : SimpleGenerator(capture, 
                      SimpleGeneratorConfig(basicConfig, g_functionPairs, g_manualFunctionMatches, g_arrayRAIIClassCustomization, 
                                            g_classFunctionAbsorbtion, g_classMethodOverride, g_typeDefaultValues, g_forceNullParameter, 
                                            g_functionGuards, g_functionNameBlacklist, g_functionNamePostfixBlacklist,
                                            TYPE_NAME_PREFIX, FUNCTION_NAME_PREFIX, ERRORCODE_TYPE_NAME, false, false),
                      templateRoot, dstPath)
  {
  }


  CaptureConfig OpenGLESGenerator::GetCaptureConfig()
  {
    std::deque<std::string> filters;
    filters.push_back(CREATE_FUNCTION);
    filters.push_back(DESTROY_FUNCTION);
    return CaptureConfig(TYPE_NAME_PREFIX, filters, g_functionParameterNameOverride, g_functionParameterTypeOverride, true);
  }
}
