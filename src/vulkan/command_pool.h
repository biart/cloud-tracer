#pragma once


#include <array>

#include <vulkan/device.h>
#include <vulkan/memory.h>
#include <vulkan/object.h>


namespace ct
{
    namespace vulkan
    {
        enum PipelineStage : std::uint32_t
        {
            TopOfPipeStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            DrawIndirectStage = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
            VertexInputStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VertexShaderStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            TesselationControlShaderStage = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
            TesselationEvaluationShaderStage = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
            GeometryShaderStage = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
            FragmentShaderStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            EarlyFragmentTestsStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            LateFragmentTestStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            ColorAttachmentOutputStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            ComputeShaderStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            TransferStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
            BottomOfPipeStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            HostStage = VK_PIPELINE_STAGE_HOST_BIT,
            AllGraphicsStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
            AllCommandsStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        };
        using PipelineStageMask = std::uint32_t;

        enum class ImageLayout : std::uint32_t
        {
            Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
            TransferSource = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            TransferDestination = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            PresentSource = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

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
            CommandBuffer(CommandBuffer&& other);

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
            CommandRecorder(CommandRecorder&& other) = delete;

            template <typename Buffer>
            void Fill(Buffer& buffer, const uint32_t data);

            template <typename T, typename SrcMemoryType, typename DstMemoryType, bool Dst0, bool Src1>
            void Transfer(const Buffer<T, SrcMemoryType, true, Dst0>& from, Buffer<T, DstMemoryType, Src1, true>& to);

            VkAccessFlags FindSuitableAccessMask(const ImageLayout layout)
            {
                switch (layout)
                {
                case ImageLayout::TransferDestination:
                    return VK_ACCESS_TRANSFER_WRITE_BIT;
                case ImageLayout::TransferSource:
                    return VK_ACCESS_TRANSFER_READ_BIT;
                default:
                    return 0u;
                }
            }

            void ImageMemoryBarrier(
                const VkImage           image,
                const ImageLayout       old_layout,
                const PipelineStageMask source_pipe,
                const ImageLayout       new_layout,
                const PipelineStageMask destination_pipe)
            {
                VkImageMemoryBarrier image_memory_barrier = {};
                image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

                image_memory_barrier.srcAccessMask = FindSuitableAccessMask(old_layout);
                image_memory_barrier.dstAccessMask = FindSuitableAccessMask(new_layout);
                image_memory_barrier.oldLayout = static_cast<VkImageLayout>(old_layout);
                image_memory_barrier.newLayout = static_cast<VkImageLayout>(new_layout);
                image_memory_barrier.image = image;
                image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                image_memory_barrier.subresourceRange.levelCount = 1;
                image_memory_barrier.subresourceRange.layerCount = 1;
                image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                vkCmdPipelineBarrier(
                    command_buffer.GetHandle(),
                    source_pipe,
                    destination_pipe,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &image_memory_barrier);
            }

            template <typename T, typename SrcMemoryType, bool Dst>
            void Blit(const Buffer<T, SrcMemoryType, true, Dst>& buffer, const VkImage image, const uint32_t width, const uint32_t height)
            {
                ImageMemoryBarrier(
                    image,
                    ImageLayout::PresentSource, TopOfPipeStage,
                    ImageLayout::TransferDestination, TransferStage);

                VkBufferImageCopy buffer_image_copy = {};
                buffer_image_copy.bufferRowLength = width; // TODO: multiply by size?
                buffer_image_copy.bufferImageHeight = height;
                buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_image_copy.imageSubresource.layerCount = 1u;
                buffer_image_copy.imageExtent.width = width;
                buffer_image_copy.imageExtent.height = height;
                buffer_image_copy.imageExtent.depth = 1u;

                vkCmdCopyBufferToImage(
                    command_buffer.GetHandle(),
                    buffer.GetBufferHandle(),
                    image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1u,
                    &buffer_image_copy);

                ImageMemoryBarrier(
                    image,
                    ImageLayout::TransferDestination, TransferStage,
                    ImageLayout::PresentSource, BottomOfPipeStage);
            }



        private:
            CommandBuffer& command_buffer;
        };


        class Semaphore;


        void SubmitCommands(
            const CommandBuffer&    command_buffer,
            const Semaphore*        signal_semaphore_ptr = nullptr);


        void SubmitCommands(
            const CommandBuffer&    command_buffer,
            const PipelineStageMask wait_stage_mask,
            const Semaphore&        wait_semaphore,
            const Semaphore*        signal_semaphore_ptr = nullptr);
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
