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

#include <evan/ObjectRegistry.hpp>

#include <cstddef>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

namespace xider::tests
{
	/**
	 * @brief Minimal object type: the registry is a template, so its logic is
	 * testable without constructing device-bound evan types.
	 */
	struct DummyObject {
		int value = 0;
	};

	using DummyRegistry = evan::ObjectRegistry<DummyObject>;

	TEST(ObjectRegistryTest, AddReturnsAHandleThatResolves)
	{
		DummyRegistry registry;
		const auto object = std::make_shared<DummyObject>();
		object->value	  = 42;

		const auto handle = registry.add(object);

		EXPECT_NE(handle, DummyRegistry::kInvalidHandle);
		ASSERT_NE(registry.get(handle), nullptr);
		EXPECT_EQ(registry.get(handle)->value, 42);
		EXPECT_EQ(registry.size(), 1u);
		EXPECT_FALSE(registry.empty());
	}

	TEST(ObjectRegistryTest, AddingNullIsRejected)
	{
		DummyRegistry registry;

		EXPECT_EQ(registry.add(nullptr), DummyRegistry::kInvalidHandle);
		EXPECT_TRUE(registry.empty());
	}

	TEST(ObjectRegistryTest, UnknownHandleResolvesToNullptr)
	{
		DummyRegistry registry;
		(void)registry.add(std::make_shared<DummyObject>());

		EXPECT_EQ(registry.get(12345), nullptr);
	}

	TEST(ObjectRegistryTest, RemoveInvalidatesTheHandle)
	{
		DummyRegistry registry;
		const auto handle = registry.add(std::make_shared<DummyObject>());

		EXPECT_TRUE(registry.remove(handle));
		EXPECT_EQ(registry.get(handle), nullptr);
		EXPECT_EQ(registry.size(), 0u);
		EXPECT_TRUE(registry.empty());
		EXPECT_FALSE(registry.remove(handle));
	}

	TEST(ObjectRegistryTest, RecycledSlotGetsAFreshHandle)
	{
		DummyRegistry registry;

		const auto first = registry.add(std::make_shared<DummyObject>());
		EXPECT_TRUE(registry.remove(first));

		const auto replacement = std::make_shared<DummyObject>();
		replacement->value	   = 7;
		const auto second	   = registry.add(replacement);

		EXPECT_NE(first, second);
		EXPECT_EQ(registry.get(first), nullptr);
		ASSERT_NE(registry.get(second), nullptr);
		EXPECT_EQ(registry.get(second)->value, 7);
	}

	TEST(ObjectRegistryTest, UpdateReplacesTheObject)
	{
		DummyRegistry registry;
		const auto handle  = registry.add(std::make_shared<DummyObject>());
		const auto updated = std::make_shared<DummyObject>();
		updated->value	   = 9;

		EXPECT_TRUE(registry.update(handle, updated));
		ASSERT_NE(registry.get(handle), nullptr);
		EXPECT_EQ(registry.get(handle)->value, 9);

		EXPECT_FALSE(registry.update(999, std::make_shared<DummyObject>()));
		EXPECT_FALSE(registry.update(handle, nullptr));
	}

	TEST(ObjectRegistryTest, ClearDropsEverythingAndInvalidatesHandles)
	{
		DummyRegistry registry;
		const auto handle = registry.add(std::make_shared<DummyObject>());

		registry.clear();

		EXPECT_TRUE(registry.empty());
		EXPECT_EQ(registry.size(), 0u);
		EXPECT_EQ(registry.get(handle), nullptr);
	}

	TEST(ObjectRegistryTest, ForEachVisitsLiveObjectsInSlotOrder)
	{
		DummyRegistry registry;

		const auto makeObject = [](int value) {
			auto object	  = std::make_shared<DummyObject>();
			object->value = value;
			return object;
		};

		const auto first  = registry.add(makeObject(1));
		const auto second = registry.add(makeObject(2));
		const auto third  = registry.add(makeObject(3));
		EXPECT_TRUE(registry.remove(second));
		const auto fourth = registry.add(makeObject(4));

		std::vector<std::size_t> handles;
		std::vector<int> values;
		registry.forEach([&](std::size_t handle,
							 const std::shared_ptr<DummyObject> &object) {
			handles.push_back(handle);
			values.push_back(object->value);
		});

		ASSERT_EQ(handles.size(), 3u);
		ASSERT_EQ(values.size(), 3u);
		// The recycled slot keeps its original position, so the third
		// registration appears between the first and the fourth.
		EXPECT_EQ(handles[0], first);
		EXPECT_EQ(handles[1], fourth);
		EXPECT_EQ(handles[2], third);
		EXPECT_EQ(values[0], 1);
		EXPECT_EQ(values[1], 4);
		EXPECT_EQ(values[2], 3);
	}

	TEST(ObjectRegistryTest, StressInterleavedAddsAndRemoves)
	{
		DummyRegistry registry;
		std::vector<std::size_t> live;
		std::vector<std::size_t> stale;

		for (int i = 0; i < 10000; ++i) {
			if (live.empty() || i % 3 != 0) {
				const auto handle =
					registry.add(std::make_shared<DummyObject>());
				ASSERT_NE(handle, DummyRegistry::kInvalidHandle);
				live.push_back(handle);
			} else {
				const std::size_t handle = live.back();
				live.pop_back();
				ASSERT_TRUE(registry.remove(handle));
				stale.push_back(handle);
			}
		}

		EXPECT_EQ(registry.size(), live.size());
		for (const std::size_t handle: live) {
			EXPECT_NE(registry.get(handle), nullptr);
		}
		for (const std::size_t handle: stale) {
			EXPECT_EQ(registry.get(handle), nullptr);
		}
	}
}	 // namespace xider::tests
