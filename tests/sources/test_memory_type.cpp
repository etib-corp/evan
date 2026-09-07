/*
 Copyright (c) 2026 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <evan/MemoryType.hpp>

#include <gtest/gtest.h>

namespace xider::tests
{
	TEST(MemoryTypeTest, FindsMatchingMemoryType)
	{
		VkPhysicalDeviceMemoryProperties properties {};
		properties.memoryTypeCount = 2;
		properties.memoryTypes[0].propertyFlags =
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		properties.memoryTypes[1].propertyFlags =
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		auto result = evan::findMemoryTypeIndex(
			properties, 0b11, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		EXPECT_TRUE(evan::isOk(result.code));
		EXPECT_EQ(result.value, 1u);
	}

	TEST(MemoryTypeTest, ReturnsErrorWhenNoMemoryTypeMatches)
	{
		VkPhysicalDeviceMemoryProperties properties {};
		properties.memoryTypeCount = 1;
		properties.memoryTypes[0].propertyFlags =
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		auto result = evan::findMemoryTypeIndex(
			properties, 0b1,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		EXPECT_FALSE(evan::isOk(result.code));
		EXPECT_TRUE(evan::isFatal(result.code));
		EXPECT_EQ(result.value, 0u);
	}

	TEST(MemoryTypeTest, RespectsTypeFilter)
	{
		VkPhysicalDeviceMemoryProperties properties {};
		properties.memoryTypeCount = 2;
		properties.memoryTypes[0].propertyFlags =
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		properties.memoryTypes[1].propertyFlags =
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		auto result = evan::findMemoryTypeIndex(
			properties, 0b10, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		EXPECT_TRUE(evan::isOk(result.code));
		EXPECT_EQ(result.value, 1u);
	}
}	 // namespace xider::tests
