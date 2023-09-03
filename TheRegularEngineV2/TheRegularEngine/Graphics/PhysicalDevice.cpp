#include "pch.h"
#include "PhysicalDevice.h"
#include "RendererContext.h"
#include "Core/Logger.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	VkPhysicalDevice PhysicalDevice::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	QueueFamilies PhysicalDevice::GetQueueFamilies()
	{
		return m_QueueFamilies;
	}

	VkPhysicalDeviceProperties PhysicalDevice::GetPhysicalDeviceProperties()
	{
		return m_Properties;
	}

	VkPhysicalDeviceMemoryProperties PhysicalDevice::GetPhysicalDeviceMemoryProperties()
	{
		return m_MemoryProperties;
	}
	
	QueueFamilies PhysicalDevice::FindQueueFamilies(VkPhysicalDevice dev)
	{
		uint32_t Queuecount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &Queuecount, nullptr); //Get number of queue by passing nullptr

		std::vector<VkQueueFamilyProperties> QueueFamilies(Queuecount);
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &Queuecount, QueueFamilies.data()); //Get the actual queues properties by passing data

		int i = 0;
		for (const auto& queuefamily : QueueFamilies)
		{
			if (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				m_QueueFamilies.Graphics = i;
			}

			VkBool32 PresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, m_Surface, &PresentSupport);

			if (PresentSupport)
			{
				m_QueueFamilies.Present = i;
			}

			if (m_QueueFamilies.IsComplete())
			{
				break;
			}
			i++;
		}

		return m_QueueFamilies;
	}

	SwapChainDetails PhysicalDevice::QuerySwapChainSupprt(VkPhysicalDevice device)
	{
		SwapChainDetails Details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &Details.Capabilities);

		uint32_t FormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &FormatCount, nullptr);

		if (FormatCount != 0)
		{
			Details.Formats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &FormatCount, Details.Formats.data());
		}

		uint32_t PresentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &PresentCount, nullptr);

		if (PresentCount != 0)
		{
			Details.PresentModes.resize(PresentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &PresentCount, Details.PresentModes.data());
		}

		return Details;
	}

	bool PhysicalDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t ExtensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, nullptr); //Get the count

		std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, AvailableExtensions.data()); //Get the properties
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::set<std::string> RequiredExtension(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

		for (const auto& extension : AvailableExtensions)
		{
			RequiredExtension.erase(extension.extensionName);
		}

		return RequiredExtension.empty();
	}

	bool PhysicalDevice::IsPhysicalDeviceSuitable(VkPhysicalDevice pd)
	{
		m_QueueFamilies = FindQueueFamilies(pd);
		bool ExtensionSupported = CheckDeviceExtensionSupport(pd);

		bool SwapChainSupported = false;
		if (ExtensionSupported)
		{
			SwapChainDetails Details = QuerySwapChainSupprt(pd);
			SwapChainSupported = !Details.Formats.empty() && !Details.PresentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(pd, &supportedFeatures);

		return m_QueueFamilies.IsComplete() && ExtensionSupported && SwapChainSupported && supportedFeatures.samplerAnisotropy;
	}

	uint32_t PhysicalDevice::GetPhysicalDeviceCount()
	{
		uint32_t PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &PhysicalDeviceCount, nullptr);
		if (PhysicalDeviceCount == 0)
		{
			assert(false);
		}
		return PhysicalDeviceCount;
	}

	PhysicalDevice::PhysicalDevice(VkSurfaceKHR Surface) : m_Surface(Surface)
	{
		uint32_t PhysicalDeviceCount = GetPhysicalDeviceCount();
		std::vector<VkPhysicalDevice> PhysicalDevice(PhysicalDeviceCount);
		vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &PhysicalDeviceCount, PhysicalDevice.data());

		std::cout << "Available GPUs:\n";
		for (const auto& device : PhysicalDevice)
		{
			VkPhysicalDeviceProperties DeviceProp;
			vkGetPhysicalDeviceProperties(device, &DeviceProp);
			std::cout << DeviceProp.deviceName << std::endl;
		}
		for (const auto& device : PhysicalDevice)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		assert(m_PhysicalDevice != VK_NULL_HANDLE);

		//auto VulkanInstance = RendererContext::GetVKInstance();

		//uint32_t GPUcount;
		//vkEnumeratePhysicalDevices(VulkanInstance, &GPUcount, nullptr);
		//if (GPUcount == 0)
		//{
		//	TRE_CORE_CRITICAL("No GPU suitable to support vulkan");
		//	assert(false);
		//}

		//std::vector<VkPhysicalDevice> PhysicalDevices(GPUcount);
		//if (auto Result = vkEnumeratePhysicalDevices(VulkanInstance, &GPUcount, PhysicalDevices.data()); Result != VK_SUCCESS)
		//{
		//	TRE_CORE_CRITICAL("Cannot get GPUs");
		//	assert(Result == VK_SUCCESS);
		//}

		////Choose GPU that is discrete and ignore integrated ones.
		////Note that this does not account for computers with more than 1 discrete GPU.
		////It will just choose the first one that was iterated.
		//for (VkPhysicalDevice PhysicalDevice : PhysicalDevices) //Pointer no const ref
		//{
		//	vkGetPhysicalDeviceProperties(PhysicalDevice, &m_Properties);
		//	if (m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		//	{
		//		m_PhysicalDevice = PhysicalDevice;
		//		break;
		//	}
		//}

		//if (m_PhysicalDevice == nullptr) //Worst case scenario
		//{
		//	TRE_CORE_WARN("No discrete GPU found, using integrated");
		//	m_PhysicalDevice = PhysicalDevices.back();
		//	assert(m_PhysicalDevice);
		//}

		//vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
		//vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

		//uint32_t ExtensionsCount;
		//vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ExtensionsCount, nullptr);
		//if (ExtensionsCount > 0)
		//{
		//	std::vector<VkExtensionProperties> Extensions(ExtensionsCount);
		//	if (vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ExtensionsCount, Extensions.data()) == VK_SUCCESS)
		//	{
		//		TRE_CORE_INFO("Supported Extensions from this GPU: ");
		//		for (const auto& Ext : Extensions)
		//		{
		//			m_SupportedExtensions.emplace(Ext.extensionName);
		//			TRE_CORE_INFO(Ext.extensionName);
		//		}
		//	}
		//}

		//uint32_t QueueFamilyCount;
		//vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, nullptr);
		//assert(QueueFamilyCount > 0);

		//m_QueueFamilyProperties.resize(QueueFamilyCount);
		//vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, m_QueueFamilyProperties.data());

		//static const float DefaultQueuePriority = 0.f;
		//int RequestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
		//m_QueueFamilies = GetQueueFamilies(RequestedQueueTypes);

		//if (RequestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		//{
		//	VkDeviceQueueCreateInfo QueueCreateInfo{};
		//	QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//	QueueCreateInfo.queueFamilyIndex = m_QueueFamilies.Graphics;
		//	QueueCreateInfo.queueCount = 1;
		//	QueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
		//	m_QueueCreateInfos.push_back(QueueCreateInfo);
		//}

		//if (RequestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		//{
		//	if (m_QueueFamilies.Compute != m_QueueFamilies.Graphics) //If same no need create twice
		//	{
		//		VkDeviceQueueCreateInfo QueueCreateInfo{};
		//		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//		QueueCreateInfo.queueFamilyIndex = m_QueueFamilies.Compute;
		//		QueueCreateInfo.queueCount = 1;
		//		QueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
		//		m_QueueCreateInfos.push_back(QueueCreateInfo);
		//	}
		//}

		//if (RequestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		//{
		//	if ((m_QueueFamilies.Compute != m_QueueFamilies.Transfer) && (m_QueueFamilies.Graphics != m_QueueFamilies.Transfer)) //If same no need create twice
		//	{
		//		VkDeviceQueueCreateInfo QueueCreateInfo{};
		//		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//		QueueCreateInfo.queueFamilyIndex = m_QueueFamilies.Transfer;
		//		QueueCreateInfo.queueCount = 1;
		//		QueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
		//		m_QueueCreateInfos.push_back(QueueCreateInfo);
		//	}
		//}

		//m_DepthFormat = GetDepthFormat();
		//assert(m_DepthFormat);
	}

	PhysicalDevice::~PhysicalDevice()
	{

	}

	QueueFamilies PhysicalDevice::GetQueueFamilies(int flags)
	{
		QueueFamilies NewFamily;

		//if (flags & VK_QUEUE_COMPUTE_BIT)
		//{
		//	for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
		//	{
		//		auto& properties = m_QueueFamilyProperties[x];
		//		if ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
		//		{
		//			NewFamily.Compute = x;
		//			break;
		//		}
		//	}
		//}

		//if (flags & VK_QUEUE_TRANSFER_BIT)
		//{
		//	for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
		//	{
		//		auto& properties = m_QueueFamilyProperties[x];
		//		if ((properties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) 
		//															&& ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
		//		{
		//			NewFamily.Compute = x;
		//			break;
		//		}
		//	}
		//}

		//for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
		//{
		//	if ((flags & VK_QUEUE_COMPUTE_BIT) && NewFamily.Compute == -1)
		//	{
		//		if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_COMPUTE_BIT)
		//			NewFamily.Compute = x;
		//	}

		///*	if ((flags & VK_QUEUE_TRANSFER_BIT) && NewFamily.Transfer == -1)
		//	{
		//		if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_TRANSFER_BIT)
		//			NewFamily.Transfer = x;
		//	}*/

		//	if (flags & VK_QUEUE_GRAPHICS_BIT)
		//	{
		//		if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		//			NewFamily.Graphics = x;
		//	}
		//}

		return NewFamily;
	}

	VkFormat PhysicalDevice::GetDepthFormat()
	{
		std::vector<VkFormat> depthFormats = 
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				return format;
		}

		return VK_FORMAT_UNDEFINED;
	}

	bool PhysicalDevice::IsExtensionSupported(const std::string& Extension)
	{
		return m_SupportedExtensions.find(Extension) != m_SupportedExtensions.end();
	}
}