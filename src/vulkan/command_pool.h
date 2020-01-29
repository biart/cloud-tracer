#pragma once


#include <array>

#include <vulkan/device.h>
#include <vulkan/memory.h>
#include <vulkan/object.h>


namespace ct
{
    namespace vulkan
    {
        class CommandPool : public Object<VkCommandPool>
        {
        public:
            explicit CommandPool(const Device& device, QueueType queue_type);
            ~CommandPool();

            const Device& GetDevice() const;
            QueueType GetQueueType() const;
            VkQueue GetQueue() const;
            void WaitForQueue() const;
            void Trim() const;

        private:
            const Device& device;
            const QueueType queue_type;
        };


        class CommandBuffer : public Object<VkCommandBuffer>
        {
            friend class CommandRecorder;

        public:
            CommandBuffer(const CommandPool& command_pool);
            ~CommandBuffer();

            const CommandPool& GetPool() const;

            enum Status
            {
                Initial,
                Recording,
                Executable,
                Pending
            };
            Status GetStatus() const;
            void Reset();

        private:
            void StartRecording();
            void StopRecording();

            const CommandPool& command_pool;
            Status status;
        };


        class CommandRecorder
        {
        public:
            CommandRecorder(CommandBuffer& command_buffer);
            ~CommandRecorder();

            template <typename Buffer>
            void Fill(Buffer& buffer, const uint32_t data);

            template <typename T, typename SrcMemoryType, typename DstMemoryType, bool Dst0, bool Src1>
            void Transfer(const Buffer<T, SrcMemoryType, true, Dst0>& from, Buffer<T, DstMemoryType, Src1, true>& to);

            template <typename T, typename SrcMemoryType, bool Dst>
            void Blit(const Buffer<T, SrcMemoryType, true, Dst>& buffer, const VkImage image, const uint32_t width, const uint32_t height)
            {
                VkImageMemoryBarrier image_memory_barrier = {};
                image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                image_memory_barrier.image = image;
                image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                image_memory_barrier.subresourceRange.levelCount = 1;
                image_memory_barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(
                    command_buffer.GetHandle(),
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &image_memory_barrier);

                VkBufferImageCopy buffer_image_copy = {};
                buffer_image_copy.bufferRowLength = width; // TODO: multiply by size?
                buffer_image_copy.bufferImageHeight = height;
                buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_image_copy.imageSubresource.layerCount = 1;
                buffer_image_copy.imageExtent.width = width;
                buffer_image_copy.imageExtent.height = height;
                buffer_image_copy.imageExtent.depth = 1;
                vkCmdCopyBufferToImage(
                    command_buffer.GetHandle(),
                    buffer.GetBufferHandle(),
                    image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1u,
                    &buffer_image_copy);

                image_memory_barrier = {};
                image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                image_memory_barrier.image = image;
                image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                image_memory_barrier.subresourceRange.levelCount = 1;
                image_memory_barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(
                    command_buffer.GetHandle(),
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &image_memory_barrier);
            }



        private:
            CommandBuffer& command_buffer;
        };


        void SubmitCommands(const CommandBuffer& command_buffer);
    }
}



template <typename Buffer>
void ct::vulkan::CommandRecorder::Fill(Buffer& buffer, const uint32_t data)
{
    vkCmdFillBuffer(command_buffer.GetHandle(), buffer.GetHandle(), 0, buffer.GetSizeInBytes(), data);
}

template <typename T, typename SrcMemoryType, typename DstMemoryType, bool Dst0, bool Src1>
void ct::vulkan::CommandRecorder::Transfer(const Buffer<T, SrcMemoryType, true, Dst0>& from, Buffer<T, DstMemoryType, Src1, true>& to)
{
    VkBufferCopy copy_region;
    copy_region.srcOffset = 0u;
    copy_region.dstOffset = 0u;
    copy_region.size = from.GetSizeInBytes();
    assert(copy_region.size == to.GetSizeInBytes());
    vkCmdCopyBuffer(command_buffer.GetHandle(), from.GetBufferHandle(), to.GetBufferHandle(), 1u, &copy_region);
}
