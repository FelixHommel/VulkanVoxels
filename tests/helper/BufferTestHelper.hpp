#ifndef VULKAN_VOXELS_TESTS_HELPER_BUFFER_TETS_HELPER_HPP
#define VULKAN_VOXELS_TESTS_HELPER_BUFFER_TETS_HELPER_HPP

#include "core/Buffer.hpp"

namespace vv::test
{

class BufferTestHelper
{
public:
    BufferTestHelper() = delete;

    static const void* getMappedMemory(const Buffer& buffer) { return buffer.m_mapped; }
};

}

#endif // !VULKAN_VOXELS_TESTS_HELPER_BUFFER_TETS_HELPER_HPP
