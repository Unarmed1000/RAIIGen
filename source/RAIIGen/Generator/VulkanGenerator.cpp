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

#include <RAIIGen/Generator/VulkanGenerator.hpp>
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
    // OpenCL
    //const auto VULKAN_CREATE_FUNCTION = "clCreate";
    //const auto VULKAN_DESTROY_FUNCTION = "clDestroy";
    //const auto VULKAN_ALLOCATE_FUNCTION = "clAllocate";
    //const auto VULKAN_FREE_FUNCTION = "clFree";


    const auto CREATE_FUNCTION = "vkCreate";
    const auto DESTROY_FUNCTION = "vkDestroy";
    const auto ALLOCATE_FUNCTION = "vkAllocate";
    const auto FREE_FUNCTION = "vkFree";

    const auto TYPE_NAME_PREFIX = "Vk";

    const auto DEFAULT_VALUE = "VK_NULL_HANDLE";



    const std::vector<FunctionNamePair> g_functionPairs
    {
      FunctionNamePair(CREATE_FUNCTION, DESTROY_FUNCTION),
      FunctionNamePair(ALLOCATE_FUNCTION, FREE_FUNCTION)
    };


    // Manual matches for methods that don't follow 'standard' patterns
    const std::vector<FunctionNamePair> g_manualFunctionMatches
    {
      // Pipelines are destroyed with vkDestroyPipeline
      FunctionNamePair("vkCreateGraphicsPipelines", "vkDestroyPipeline"),
      FunctionNamePair("vkCreateComputePipelines", "vkDestroyPipeline"),
      FunctionNamePair("vkCreateDisplayPlaneSurfaceKHR", "vkDestroySurfaceKHR"),
    };


    const std::vector<ClassFunctionAbsorb> g_classFunctionAbsorbtion
    {
    };


    const std::vector<std::string> g_forceNullParameter
    {
      "VkAllocationCallbacks"
    };


    //! Not necessary for the Vulkan header as it was properly constructed!
    const std::vector<FunctionParameterNameOverride> g_functionParameterNameOverride
    {
    };


    const std::unordered_map<std::string, std::string> g_typeDefaultValues =
    {
      { "VkBuffer", DEFAULT_VALUE },
      { "VkBufferView", DEFAULT_VALUE },
      { "VkCommandBuffer", DEFAULT_VALUE },
      { "VkCommandPool", DEFAULT_VALUE },
      { "VkDebugReportCallbackEXT", DEFAULT_VALUE },
      { "VkDescriptorPool", DEFAULT_VALUE },
      { "VkDescriptorSet", DEFAULT_VALUE },
      { "VkDescriptorSetLayout", DEFAULT_VALUE },
      { "VkDevice", DEFAULT_VALUE },
      { "VkDeviceMemory", DEFAULT_VALUE },
      { "VkEvent", DEFAULT_VALUE },
      { "VkFence", DEFAULT_VALUE },
      { "VkFramebuffer", DEFAULT_VALUE },
      { "VkImage", DEFAULT_VALUE },
      { "VkImageView", DEFAULT_VALUE },
      { "VkInstance", DEFAULT_VALUE },
      { "VkPipeline", DEFAULT_VALUE },
      { "VkPipelineCache", DEFAULT_VALUE },
      { "VkPipelineLayout", DEFAULT_VALUE },
      { "VkQueryPool", DEFAULT_VALUE },
      { "VkRenderPass", DEFAULT_VALUE },
      { "VkSampler", DEFAULT_VALUE },
      { "VkSemaphore", DEFAULT_VALUE },
      { "VkShaderModule", DEFAULT_VALUE },
      { "VkSurfaceKHR", DEFAULT_VALUE },
      { "VkSwapchainKHR", DEFAULT_VALUE },
    };
  }


  VulkanGenerator::VulkanGenerator(const Capture& capture, const BasicConfig& basicConfig, const Fsl::IO::Path& templateRoot, const Fsl::IO::Path& dstPath)
    : SimpleGenerator(capture, SimpleGeneratorConfig(basicConfig, g_functionPairs, g_manualFunctionMatches, g_classFunctionAbsorbtion, g_typeDefaultValues, g_forceNullParameter, TYPE_NAME_PREFIX), templateRoot, dstPath)
  {
  }


  CaptureConfig VulkanGenerator::GetCaptureConfig()
  {
    std::deque<std::string> filters;
    //filters.push_back(CREATE_FUNCTION);
    //filters.push_back(DESTROY_FUNCTION);
    //filters.push_back(ALLOCATE_FUNCTION);
    //filters.push_back(FREE_FUNCTION);
    return CaptureConfig(TYPE_NAME_PREFIX, filters, g_functionParameterNameOverride, true);
  }
}