    VkMemoryRequirements GetBufferMemoryRequirements() const
    {
      VkMemoryRequirements requirements;
      vkGetBufferMemoryRequirements(m_device, m_buffer, &requirements);
      return requirements;
    }