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

#include <evan/GPUMaterial.hpp>

#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace xider::tests
{
	static_assert(
		std::is_reference_v<decltype(std::declval<const evan::GPUMaterial &>()
										 .getDescriptorSets())>,
		"GPUMaterial::getDescriptorSets() must return a const reference to "
		"avoid per-bind vector copies (PERFORMANCE_AUDIT_EVAN.md P1.4)");

	TEST(GPUMaterial, GetDescriptorSetsReturnsConstReference)
	{
		EXPECT_TRUE(
			(std::is_reference_v<decltype(
				std::declval<const evan::GPUMaterial &>()
					.getDescriptorSets())>));
	}
}	 // namespace xider::tests
