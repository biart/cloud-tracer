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

        template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
        class Buffer
        {
        public:
            explicit Buffer(const Device& device, const std::size_t count);
            ~Buffer();
            Buffer(Buffer<T, MemoryType, UsageFlags>&& other);
            Buffer(const Buffer<T, MemoryType, UsageFlags>& other) = delete;

            std::size_t GetCount() const;
            std::size_t GetAllocationSizeInBytes() const;
            std::size_t GetSizeInBytes() const;

            VkBuffer GetBufferHandle() const;
            VkDeviceMemory GetMemoryHandle() const;
            const Device& GetDevice() const;

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
            VkDeviceMemory      vk_memory = VK_NULL_HANDLE;
            VkBuffer            vk_buffer = VK_NULL_HANDLE;
        };


        // Shortcuts for some buffer types
        template <typename T>
        using StagingBuffer = Buffer<T, HostMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT>;

        template <typename T>
        using DeviceBuffer = Buffer<T, DeviceMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT>;

        template <typename T>
        using UniformBuffer = Buffer<T, HostMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT>;


        template <typename T>
        class MemoryMap
        {
        public:
            explicit MemoryMap(const StagingBuffer<T>& buffer);
            MemoryMap(const MemoryMap<T>& other) = delete;
            MemoryMap(MemoryMap<T>&& other);
            ~MemoryMap();

            T& operator[](const std::size_t index) const;
            T* begin() const;
            T* end() const;
            std::size_t GetCount() const;

        private:
            const Device&           device;
            const VkDeviceMemory    vk_memory;

            mutable T*          data;
            std::size_t         count;
        };


        template <typename T>
        MemoryMap<T> MapMemory(const StagingBuffer<T>& buffer);
    }
}



template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
ct::vulkan::Buffer<T, MemoryType, UsageFlags>::Buffer(
    const ct::vulkan::Device&   device,
    const std::size_t           count) :
    device(device),
    count(count)
{
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = count * sizeof(T);
    buffer_create_info.usage = UsageFlags;
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

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline ct::vulkan::Buffer<T, MemoryType, UsageFlags>::~Buffer()
{
    if (vk_buffer != VK_NULL_HANDLE)
    {
        vkFreeMemory(device.GetHandle(), vk_memory, nullptr);
        vkDestroyBuffer(device.GetHandle(), vk_buffer, nullptr);
    }
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline ct::vulkan::Buffer<T, MemoryType, UsageFlags>::Buffer(
    Buffer<T, MemoryType, UsageFlags>&& other) :
    device(other.device),
    count(other.count),
    allocation_size(other.allocation_size),
    vk_memory(other.vk_memory)
{
    swap(vk_buffer, other.vk_buffer);
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline std::size_t ct::vulkan::Buffer<T, MemoryType, UsageFlags>::GetCount() const
{
    return count;
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline std::size_t ct::vulkan::Buffer<T, MemoryType, UsageFlags>::GetAllocationSizeInBytes() const
{
    return allocation_size;
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline std::size_t ct::vulkan::Buffer<T, MemoryType, UsageFlags>::GetSizeInBytes() const
{
    return count * sizeof(T);
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline VkBuffer ct::vulkan::Buffer<T, MemoryType, UsageFlags>::GetBufferHandle() const
{
    return vk_buffer;
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline VkDeviceMemory ct::vulkan::Buffer<T, MemoryType, UsageFlags>::GetMemoryHandle() const
{
    return vk_memory;
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline const ct::vulkan::Device& ct::vulkan::Buffer<T, MemoryType, UsageFlags>::GetDevice() const
{
    return device;
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline std::uint32_t ct::vulkan::Buffer<T, MemoryType, UsageFlags>::FindSuitableMemoryTypeIndex(
    const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties,
    const VkMemoryRequirements&             memory_requirements,
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

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline bool ct::vulkan::Buffer<T, MemoryType, UsageFlags>::IsMemorySupportedByDevice(
    const std::uint32_t             memory_type_index,
    const VkMemoryRequirements&     memory_requirements)
{
    return (memory_requirements.memoryTypeBits & (1 << memory_type_index)) != 0;
}

template <typename T, typename MemoryType, VkBufferUsageFlags UsageFlags>
inline bool ct::vulkan::Buffer<T, MemoryType, UsageFlags>::DoesMemoryHaveProperties(
    const VkMemoryType&             memory_type,
    const VkMemoryPropertyFlags     requested_memory_properties)
{
    return (memory_type.propertyFlags & requested_memory_properties) == requested_memory_properties;
}



template <typename T>
inline ct::vulkan::MemoryMap<T>::MemoryMap(const StagingBuffer<T>& buffer) :
    device(buffer.GetDevice()),
    vk_memory(buffer.GetMemoryHandle()),
    count(buffer.GetCount())
{
    if (vkMapMemory(device.GetHandle(), vk_memory, 0u, count * sizeof(T), 0u, &static_cast<void*>(data)) != VK_SUCCESS)
    {
        throw Exception("Failed to map host buffer memory");
    }
}

template<typename T>
inline ct::vulkan::MemoryMap<T>::MemoryMap(MemoryMap<T>&& other) :
    device(other.device),
    vk_memory(other.vk_memory),
    data(other.data),
    count(other.count)
{
}

template <typename T>
inline ct::vulkan::MemoryMap<T>::~MemoryMap()
{
    vkUnmapMemory(device.GetHandle(), vk_memory);
}

template <typename T>
inline T& ct::vulkan::MemoryMap<T>::operator[](const std::size_t index) const
{
    assert(index < count);
    return data[index];
}

template <typename T>
inline T* ct::vulkan::MemoryMap<T>::begin() const
{
    return data;
}

template <typename T>
inline T* ct::vulkan::MemoryMap<T>::end() const
{
    return data + count;
}

template <typename T>
inline std::size_t ct::vulkan::MemoryMap<T>::GetCount() const
{
    return count;
}

template <typename T>
inline ct::vulkan::MemoryMap<T> ct::vulkan::MapMemory(const StagingBuffer<T>& buffer)
{
    return MemoryMap<T>(buffer);
}
