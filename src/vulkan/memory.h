#pragma once


#include <vulkan/device.h>
#include <vulkan/object.h>


namespace ct
{
    namespace vulkan
    {
        struct HostMemory
        {
            static constexpr VkMemoryPropertyFlags vk_memory_property_flags =
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            static constexpr const char* name = "host";
        };

        struct DeviceMemory
        {
            static constexpr VkMemoryPropertyFlags vk_memory_property_flags =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            static constexpr const char* name = "device";
        };

        template <typename T, typename MemoryType, bool Source = false, bool Destination = false>
        class Buffer
        {
        public:
            Buffer(const Device& device, const std::size_t count);
            ~Buffer();

            std::size_t GetCount() const;
            std::size_t GetAllocationSizeInBytes() const;
            std::size_t GetSizeInBytes() const;

            VkBuffer GetBufferHandle() const;
            VkDeviceMemory GetMemoryHandle() const;

        private:
            static std::uint32_t FindSuitableMemoryTypeIndex(
                const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties,
                const VkMemoryRequirements&             memory_requirements,
                const VkMemoryPropertyFlags             requested_memory_properties);

            static bool IsMemorySupportedByDevice(
                const std::uint32_t             memory_type_index,
                const VkMemoryRequirements&     memory_requirements);

            static bool DoesMemoryHaveProperties(
                const VkMemoryType&             memory_type,
                const VkMemoryPropertyFlags     requested_memory_properties);

            const Device&       device;
            const std::size_t   count;
            std::size_t         allocation_size;
            VkDeviceMemory      vk_memory;
            VkBuffer            vk_buffer;
        };
    }
}



template <typename T, typename MemoryType, bool Source, bool Destination>
ct::vulkan::Buffer<T, MemoryType, Source, Destination>::Buffer(const ct::vulkan::Device& device, const std::size_t count) :
    device(device),
    count(count)
{
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = count * sizeof(T);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (Source)         buffer_create_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (Destination)    buffer_create_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device.GetHandle(), &buffer_create_info, nullptr, &vk_buffer) != VK_SUCCESS)
    {
        throw Exception("Cannot create buffer");
    }
    auto vk_buffer_scoped = MakeScoped(vk_buffer, [vk_device = device.GetHandle()](VkBuffer& buffer)
    {
        vkDestroyBuffer(vk_device, buffer, nullptr);
    });

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device.GetHandle(), vk_buffer, &memory_requirements);

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device.GetPhysicalDevice(), &memory_properties);

    const std::uint32_t memory_type_index = FindSuitableMemoryTypeIndex(
        memory_properties,
        memory_requirements,
        MemoryType::vk_memory_property_flags);

    allocation_size = memory_requirements.size;

    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index;
    if (vkAllocateMemory(device.GetHandle(), &allocate_info, nullptr, &vk_memory) != VK_SUCCESS)
    {
        throw Exception(std::string("Cannot allocate ") + MemoryType::name + " memory");
    }

    vkBindBufferMemory(device.GetHandle(), vk_buffer, vk_memory, 0u);

    vk_buffer_scoped.Release();
}


template <typename T, typename MemoryType, bool Source, bool Destination>
ct::vulkan::Buffer<T, MemoryType, Source, Destination>::~Buffer()
{
    vkFreeMemory(device.GetHandle(), vk_memory, nullptr);
    vkDestroyBuffer(device.GetHandle(), vk_buffer, nullptr);
}


template <typename T, typename MemoryType, bool Source, bool Destination>
std::size_t ct::vulkan::Buffer<T, MemoryType, Source, Destination>::GetCount() const
{
    return count;
}

template <typename T, typename MemoryType, bool Source, bool Destination>
std::size_t ct::vulkan::Buffer<T, MemoryType, Source, Destination>::GetAllocationSizeInBytes() const
{
    return allocation_size;
}

template <typename T, typename MemoryType, bool Source, bool Destination>
std::size_t ct::vulkan::Buffer<T, MemoryType, Source, Destination>::GetSizeInBytes() const
{
    return count * sizeof(T);
}


template <typename T, typename MemoryType, bool Source, bool Destination>
VkBuffer ct::vulkan::Buffer<T, MemoryType, Source, Destination>::GetBufferHandle() const
{
    return vk_buffer;
}

template <typename T, typename MemoryType, bool Source, bool Destination>
VkDeviceMemory ct::vulkan::Buffer<T, MemoryType, Source, Destination>::GetMemoryHandle() const
{
    return vk_memory;
}

template <typename T, typename MemoryType, bool Source, bool Destination>
std::uint32_t ct::vulkan::Buffer<T, MemoryType, Source, Destination>::FindSuitableMemoryTypeIndex(
    const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties,
    const VkMemoryRequirements& memory_requirements,
    const VkMemoryPropertyFlags             requested_memory_properties)
{
    const std::uint32_t memory_type_count = physical_device_memory_properties.memoryTypeCount;
    for (std::uint32_t index = 0u; index != memory_type_count; ++index)
    {
        const VkMemoryType& memory_type = physical_device_memory_properties.memoryTypes[index];
        if (IsMemorySupportedByDevice(index, memory_requirements) &&
            DoesMemoryHaveProperties(memory_type, requested_memory_properties))
        {
            return index;
        }
    }
    throw Exception("Cannot find a memory type with requested properties");
}

template <typename T, typename MemoryType, bool Source, bool Destination>
bool ct::vulkan::Buffer<T, MemoryType, Source, Destination>::IsMemorySupportedByDevice(
    const std::uint32_t             memory_type_index,
    const VkMemoryRequirements& memory_requirements)
{
    return (memory_requirements.memoryTypeBits & (1 << memory_type_index)) != 0;
}

template <typename T, typename MemoryType, bool Source, bool Destination>
bool ct::vulkan::Buffer<T, MemoryType, Source, Destination>::DoesMemoryHaveProperties(
    const VkMemoryType& memory_type,
    const VkMemoryPropertyFlags     requested_memory_properties)
{
    return (memory_type.propertyFlags & requested_memory_properties) == requested_memory_properties;
}
