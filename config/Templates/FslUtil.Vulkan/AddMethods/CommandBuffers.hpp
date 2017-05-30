      void Begin(const std::size_t index,
                 const VkCommandBufferUsageFlags flags, const VkRenderPass renderPass, const uint32_t subpass,
                 const VkFramebuffer framebuffer, const VkBool32 occlusionQueryEnable, const VkQueryControlFlags queryFlags,
                 const VkQueryPipelineStatisticFlags pipelineStatistics)
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

        Begin(index, commandBufferBeginInfo);
      }

      void Begin(const std::size_t index, const VkCommandBufferBeginInfo& commandBufferBeginInfo)
      {
        if (index >= m_commandBuffers.size() || m_commandBuffers[index] == VK_NULL_HANDLE)
          throw UsageErrorException("Index must be valid and/or can not call Begin on a NULL handle");

        Util::Check(vkBeginCommandBuffer(m_commandBuffers[index], &commandBufferBeginInfo), "vkBeginCommandBuffer", __FILE__, __LINE__);
      }

      void End(const std::size_t index)
      {
        if (index >= m_commandBuffers.size() || m_commandBuffers[index] == VK_NULL_HANDLE)
          throw UsageErrorException("Index must be valid and/or can not call End on a NULL handle");

        Util::Check(vkEndCommandBuffer(m_commandBuffers[index]), "vkEndCommandBuffer", __FILE__, __LINE__);
      }

      void CmdBeginRenderPass(const std::size_t index, const VkRenderPassBeginInfo* pRenderPassBeginInfo, const VkSubpassContents contents)
      {
        if (index >= m_commandBuffers.size() || m_commandBuffers[index] == VK_NULL_HANDLE)
          throw UsageErrorException("Index must be valid and/or can not call CmdBeginRenderPass on a NULL handle");

        vkCmdBeginRenderPass(m_commandBuffers[index], pRenderPassBeginInfo, contents);
      }

      void CmdEndRenderPass(const std::size_t index)
      {
        if (index >= m_commandBuffers.size() || m_commandBuffers[index] == VK_NULL_HANDLE)
          throw UsageErrorException("Index must be valid and/or can not call CmdEndRenderPass on a NULL handle");

        vkCmdEndRenderPass(m_commandBuffers[index]);
      }