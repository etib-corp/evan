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

#include <evan/GPUShader.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace xider::tests
{
	namespace
	{
		constexpr uint32_t kMagic	  = 0x07230203u;
		constexpr uint32_t kVersion	  = 0x00010000u;
		constexpr uint32_t kGenerator = 0u;
		constexpr uint32_t kBound	  = 64u;
		constexpr uint32_t kSchema	  = 0u;

		constexpr uint16_t kOpVariable = 59;
		constexpr uint16_t kOpDecorate = 71;

		constexpr uint32_t kDecorationLocation	= 30;
		constexpr uint32_t kStorageClassInput	= 1;
		constexpr uint32_t kStorageClassUniform = 2;

		/**
		 * @brief Encodes an instruction word from its opcode and length.
		 */
		uint32_t encode(uint16_t opcode, uint32_t wordCount)
		{
			return (wordCount << 16) | static_cast<uint32_t>(opcode);
		}

		/**
		 * @brief Appends an `OpVariable` with the given id and storage class.
		 */
		void appendVariable(std::vector<uint32_t> &code, uint32_t id,
							uint32_t storageClass)
		{
			code.push_back(encode(kOpVariable, 4));
			code.push_back(0);	  // result type
			code.push_back(id);
			code.push_back(storageClass);
		}

		/**
		 * @brief Appends an `OpDecorate ... Location ...` instruction.
		 */
		void appendLocation(std::vector<uint32_t> &code, uint32_t id,
							uint32_t location)
		{
			code.push_back(encode(kOpDecorate, 4));
			code.push_back(id);
			code.push_back(kDecorationLocation);
			code.push_back(location);
		}

		/**
		 * @brief Builds a module header followed by @p code.
		 */
		std::vector<uint32_t> withHeader(std::vector<uint32_t> code)
		{
			std::vector<uint32_t> module { kMagic, kVersion, kGenerator, kBound,
										   kSchema };

			module.insert(module.end(), code.begin(), code.end());
			return module;
		}

		/**
		 * @brief Builds the input interface of `mesh.vert`: positions, colors
		 * and the four columns of the per-instance matrix.
		 */
		std::vector<uint32_t> instancedInterface()
		{
			std::vector<uint32_t> code;

			appendVariable(code, 1, kStorageClassInput);
			appendVariable(code, 2, kStorageClassInput);
			appendVariable(code, 3, kStorageClassInput);
			appendVariable(code, 4, kStorageClassInput);
			appendVariable(code, 5, kStorageClassInput);
			appendVariable(code, 6, kStorageClassInput);
			appendLocation(code, 1, 0);
			appendLocation(code, 2, 1);
			appendLocation(code, 3, 3);
			appendLocation(code, 4, 4);
			appendLocation(code, 5, 5);
			appendLocation(code, 6, 6);
			return code;
		}

		/**
		 * @brief Builds the input interface of `default.vert`: positions,
		 * colors and texture coordinates, no instance matrix.
		 */
		std::vector<uint32_t> plainInterface()
		{
			std::vector<uint32_t> code;

			appendVariable(code, 1, kStorageClassInput);
			appendVariable(code, 2, kStorageClassInput);
			appendVariable(code, 3, kStorageClassInput);
			appendLocation(code, 1, 0);
			appendLocation(code, 2, 1);
			appendLocation(code, 3, 2);
			return code;
		}
	}	 // namespace

	TEST(GPUShaderInstancing, ShaderWithInstanceMatrixIsBatched)
	{
		EXPECT_TRUE(evan::GPUShader::declaresInstanceInputs(
			withHeader(instancedInterface())));
	}

	TEST(GPUShaderInstancing, ShaderWithoutInstanceMatrixIsNeverBatched)
	{
		EXPECT_FALSE(evan::GPUShader::declaresInstanceInputs(
			withHeader(plainInterface())));
	}

	TEST(GPUShaderInstancing, PartialInstanceMatrixIsNeverBatched)
	{
		std::vector<uint32_t> code;

		appendVariable(code, 1, kStorageClassInput);
		appendVariable(code, 2, kStorageClassInput);
		appendVariable(code, 3, kStorageClassInput);
		appendVariable(code, 4, kStorageClassInput);
		appendLocation(code, 1, 3);
		appendLocation(code, 2, 4);
		appendLocation(code, 3, 5);

		EXPECT_FALSE(evan::GPUShader::declaresInstanceInputs(withHeader(code)));
	}

	TEST(GPUShaderInstancing, LocationsOutsideTheInstanceRangeAreIgnored)
	{
		std::vector<uint32_t> code;

		appendVariable(code, 1, kStorageClassInput);
		appendVariable(code, 2, kStorageClassInput);
		appendVariable(code, 3, kStorageClassInput);
		appendVariable(code, 4, kStorageClassInput);
		appendLocation(code, 1, 7);
		appendLocation(code, 2, 8);
		appendLocation(code, 3, 9);
		appendLocation(code, 4, 10);

		EXPECT_FALSE(evan::GPUShader::declaresInstanceInputs(withHeader(code)));
	}

	TEST(GPUShaderInstancing, UniformAtAnInstanceLocationIsNotAnInstanceInput)
	{
		std::vector<uint32_t> code;

		// A storage class other than Input carries no per-vertex data, so its
		// location must not be mistaken for the instance matrix.
		appendVariable(code, 1, kStorageClassUniform);
		appendVariable(code, 2, kStorageClassUniform);
		appendVariable(code, 3, kStorageClassUniform);
		appendVariable(code, 4, kStorageClassUniform);
		appendLocation(code, 1, 3);
		appendLocation(code, 2, 4);
		appendLocation(code, 3, 5);
		appendLocation(code, 4, 6);

		EXPECT_FALSE(evan::GPUShader::declaresInstanceInputs(withHeader(code)));
	}

	TEST(GPUShaderInstancing, DecorationsMayPrecedeTheVariables)
	{
		std::vector<uint32_t> code;

		appendLocation(code, 4, 3);
		appendLocation(code, 5, 4);
		appendLocation(code, 6, 5);
		appendLocation(code, 7, 6);
		appendVariable(code, 4, kStorageClassInput);
		appendVariable(code, 5, kStorageClassInput);
		appendVariable(code, 6, kStorageClassInput);
		appendVariable(code, 7, kStorageClassInput);

		EXPECT_TRUE(evan::GPUShader::declaresInstanceInputs(withHeader(code)));
	}

	TEST(GPUShaderInstancing, EmptyModuleIsNeverBatched)
	{
		EXPECT_FALSE(evan::GPUShader::declaresInstanceInputs({}));
	}

	TEST(GPUShaderInstancing, ModuleWithoutHeaderIsNeverBatched)
	{
		std::vector<uint32_t> code = instancedInterface();

		EXPECT_FALSE(evan::GPUShader::declaresInstanceInputs(code));
	}

	TEST(GPUShaderInstancing, TruncatedModuleIsNeverBatched)
	{
		std::vector<uint32_t> code = withHeader(instancedInterface());

		// Keep the header and cut the first instruction: its declared length
		// runs past the end of the module, so the module is malformed and must
		// not be trusted.
		code.resize(7);

		EXPECT_FALSE(evan::GPUShader::declaresInstanceInputs(code));
	}
}	 // namespace xider::tests
