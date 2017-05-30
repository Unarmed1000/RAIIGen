    VkMemoryRequirements GetImageMemoryRequirements() const
    {
      VkMemoryRequirements requirements;
      vkGetImageMemoryRequirements(m_device, m_image, &requirements);
      return requirements;
    }


    VkSubresourceLayout GetImageSubresourceLayout(const VkImageSubresource& imageSubresource) const
    {
      VkSubresourceLayout subresourceLayout{};
      vkGetImageSubresourceLayout(m_device, m_image, &imageSubresource, &subresourceLayout);
      return subresourceLayout;
    }