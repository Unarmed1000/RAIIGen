    //! @note  Function: vkGetFenceStatus
    VkResult GetFenceStatus() const
    {
      RAPIDVULKAN_LOG_DEBUG_WARNING_IF(m_device == VK_NULL_HANDLE || m_fence == VK_NULL_HANDLE, "Fence: GetFenceStatus called on a VK_NULL_HANDLE");
      return vkGetFenceStatus(m_device, m_fence);
    }


    void WaitForFence(const uint64_t timeout)
    {
      CheckError(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout), "vkWaitForFences", __FILE__, __LINE__);
    }


    VkResult TryWaitForFence(const uint64_t timeout)
    {
      return vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout);
    }


    void ResetFence()
    {
      CheckError(vkResetFences(m_device, 1, &m_fence), "vkResetFences", __FILE__, __LINE__);
    }