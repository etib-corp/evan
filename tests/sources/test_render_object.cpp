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

#include <evan/GPUMesh.hpp>
#include <evan/RenderObject.hpp>

#include <type_traits>

#include <gtest/gtest.h>

namespace xider::tests
{
	static_assert(
		std::is_same_v<
			decltype(std::declval<evan::RenderObject &>().updateMeshes(
				std::declval<const std::map<uint32_t,
											 utility::graphic::Mesh> &>())),
			bool>,
		"RenderObject::updateMeshes() must return bool so callers can fall "
		"back to a rebuild when the topology changed");

	static_assert(
		std::is_same_v<decltype(std::declval<const evan::GPUMesh &>()
									.getVertexCount()),
					   size_t>,
		"GPUMesh::getVertexCount() must expose the allocated vertex count as "
		"size_t so updateMeshes() can validate in-place updates");

	TEST(RenderObject, UpdateMeshesReturnsBool)
	{
		EXPECT_TRUE(true);
	}

	TEST(GPUMesh, GetVertexCountReturnsSizeT)
	{
		EXPECT_TRUE(true);
	}
}	 // namespace xider::tests
