    void Begin(const VkCommandBufferUsageFlags flags, const VkRenderPass renderPass, const uint32_t subpass,
               const VkFramebuffer framebuffer, const VkBool32 occlusionQueryEnable, const VkQueryControlFlags queryFlags,
               const VkQueryPipelineStatisticFlags pipelineStatistics, const uint32_t bufferIndex = 0)
    {
      VkCommandBufferInheritanceInfo commandBufferInheritanceInfo{};
      commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
      commandBufferInheritanceInfo.renderPass = renderPass;
      commandBufferInheritanceInfo.subpass = subpass;
      commandBufferInheritanceInfo.framebuffer = framebuffer;
      commandBufferInheritanceInfo.occlusionQueryEnable = occlusionQueryEnable;
      commandBufferInheritanceInfo.queryFlags = queryFlags;
      commandBufferInheritanceInfo.pipelineStatistics = pipelineStatistics;

      VkCommandBufferBeginInfo commandBufferBeginInfo{};
      commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      commandBufferBeginInfo.flags = flags;
      commandBufferBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;

      Begin(commandBufferBeginInfo);
    }


    void Begin(const VkCommandBufferBeginInfo& commandBufferBeginInfo)
    {
      if (m_commandBuffers == VK_NULL_HANDLE)
        throw VulkanUsageErrorException("Can not call Begin on a NULL handle");

      Util::Check(vkBeginCommandBuffer(m_commandBuffers, &commandBufferBeginInfo), "vkBeginCommandBuffer", __FILE__, __LINE__);
    }


    void End()
    {
      if (m_commandBuffers == VK_NULL_HANDLE)
        throw VulkanUsageErrorException("Can not call End on a NULL handle");

      Util::Check(vkEndCommandBuffer(m_commandBuffers), "vkEndCommandBuffer", __FILE__, __LINE__);
    }


    void CmdBeginRenderPass(const VkRenderPassBeginInfo* pRenderPassBeginInfo, const VkSubpassContents contents)
    {
      vkCmdBeginRenderPass(m_commandBuffers, pRenderPassBeginInfo, contents);
    }

    void CmdEndRenderPass()
    {
      vkCmdEndRenderPass(m_commandBuffers);
    }