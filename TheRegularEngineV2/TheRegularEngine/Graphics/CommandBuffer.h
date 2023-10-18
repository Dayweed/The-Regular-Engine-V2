#pragma once

namespace TRE
{
	class CommandBuffer
	{
		public:
			CommandBuffer(std::string DebugName);
			CommandBuffer(std::string DebugName, bool SwapChain);
			~CommandBuffer();

			void Begin();
			void End();
			void Submit();

			VkCommandBuffer GetInUseCommandBuffer();
			VkCommandBuffer GetCommandBuffer(uint32_t Index);

		private:
			std::string m_DebugName;
			VkCommandPool m_CommandPool = nullptr;
			std::vector<VkCommandBuffer> m_Commandbuffers;
			VkCommandBuffer m_InUseCommandBuffer = nullptr;

			std::vector<VkFence> m_Fences;

			bool m_SwapChainCB = false; //Check if the command buffer is the swapchain's one
	};
}