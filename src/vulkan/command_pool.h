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
            CommandPool(const Device& device, QueueType queue_type);
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
