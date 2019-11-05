#if !defined(RAPIDVULKAN_DISABLE_UNROLLED_STRUCT_METHODS) && !defined(RAPIDVULKAN_DISABLE_ARRAY_METHODS)
    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    template <std::size_t TSize>
    ShaderModule(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::array<uint32_t, TSize>& code)
      : ShaderModule()
    {
      Reset(device, flags, code.size(), code.data());
    }

    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    template <std::size_t TSize>
    ShaderModule(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::array<uint8_t, TSize>& code)
      : ShaderModule()
    {
      Reset(device, flags, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
    }

    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    template <std::size_t TSize>
    void Reset(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::array<uint32_t, TSize>& code)
    {
      Reset(device, flags, code.size(), code.data());
    }

    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    template <std::size_t TSize>
    void Reset(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::array<uint8_t, TSize>& code)
    {
      Reset(device, flags, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
    }
#endif


#if !defined(RAPIDVULKAN_DISABLE_UNROLLED_STRUCT_METHODS) && !defined(RAPIDVULKAN_DISABLE_VECTOR_METHODS)
    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    ShaderModule(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::vector<uint32_t>& code)
      : ShaderModule()
    {
      Reset(device, flags, code.size(), code.data());
    }

    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    ShaderModule(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::vector<uint8_t>& code)
      : ShaderModule()
    {
      Reset(device, flags, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
    }

    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    void Reset(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::vector<uint32_t>& code)
    {
      Reset(device, flags, code.size(), code.data());
    }

    //! @brief Destroys any owned resources and then creates the requested one
    //! @note  Function: vkCreateShaderModule
    void Reset(const VkDevice device, const VkShaderModuleCreateFlags flags, const std::vector<uint8_t>& code)
    {
      Reset(device, flags, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
    }
#endif
