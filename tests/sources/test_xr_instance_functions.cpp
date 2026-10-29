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

#ifdef __OPENXR__

	#include <evan/openxr/XrExtensions.hpp>

	#include <gtest/gtest.h>

	#include <string_view>

namespace
{
	void XRAPI_CALL dummyFunction(void)
	{
	}
}	 // namespace

namespace xider::tests
{
	TEST(XrInstanceFunctionsTest, NullInstanceReportsMissing)
	{
		evan::XrExtensionLoadReport report;
		XrInstance instance = XR_NULL_HANDLE;

		const auto functions =
			evan::XrExtensions::loadInstanceFunctions(instance, &report);

		EXPECT_FALSE(functions.hasPassthrough());
		EXPECT_FALSE(functions.hasHandTracking());
		EXPECT_FALSE(functions.hasHandMesh());
		EXPECT_FALSE(functions.hasScene());
		EXPECT_FALSE(functions.hasSpatialEntity());
		EXPECT_FALSE(functions.hasSpatialQuery());
		EXPECT_FALSE(functions.hasSpatialStorage());
		EXPECT_TRUE(report.loaded.empty());
		EXPECT_FALSE(report.missing.empty());
	}

	TEST(XrInstanceFunctionsTest, ResolverLoadsAllFunctions)
	{
		const evan::XrExtensions::ProcAddrResolver resolver =
			[](const char *, PFN_xrVoidFunction *function) {
				*function =
					reinterpret_cast<PFN_xrVoidFunction>(&dummyFunction);
				return XR_SUCCESS;
			};
		evan::XrExtensionLoadReport report;

		const auto functions =
			evan::XrExtensions::loadInstanceFunctions(resolver, &report);

		EXPECT_TRUE(functions.hasPassthrough());
		EXPECT_TRUE(functions.hasHandTracking());
		EXPECT_TRUE(functions.hasHandMesh());
		EXPECT_TRUE(functions.hasScene());
		EXPECT_TRUE(functions.hasSpatialEntity());
		EXPECT_TRUE(functions.hasSpatialQuery());
		EXPECT_TRUE(functions.hasSpatialStorage());
		EXPECT_FALSE(report.loaded.empty());
		EXPECT_TRUE(report.missing.empty());
	}

	TEST(XrInstanceFunctionsTest, MissingFunctionsStayNull)
	{
		const evan::XrExtensions::ProcAddrResolver resolver =
			[](const char *, PFN_xrVoidFunction *function) {
				*function = nullptr;
				return XR_SUCCESS;
			};
		evan::XrExtensionLoadReport report;

		const auto functions =
			evan::XrExtensions::loadInstanceFunctions(resolver, &report);

		EXPECT_FALSE(functions.hasPassthrough());
		EXPECT_FALSE(functions.hasHandTracking());
		EXPECT_FALSE(functions.hasHandMesh());
		EXPECT_TRUE(report.loaded.empty());
		EXPECT_FALSE(report.missing.empty());
	}

	TEST(XrInstanceFunctionsTest, PartialLoadKeepsMissingGroupsUnavailable)
	{
		const evan::XrExtensions::ProcAddrResolver resolver =
			[](const char *name, PFN_xrVoidFunction *function) {
				if (std::string_view(name).find("Passthrough")
					!= std::string_view::npos) {
					*function =
						reinterpret_cast<PFN_xrVoidFunction>(&dummyFunction);
				} else {
					*function = nullptr;
				}
				return XR_SUCCESS;
			};
		evan::XrExtensionLoadReport report;

		const auto functions =
			evan::XrExtensions::loadInstanceFunctions(resolver, &report);

		EXPECT_TRUE(functions.hasPassthrough());
		EXPECT_FALSE(functions.hasHandTracking());
		EXPECT_FALSE(functions.hasSpatialEntity());
		EXPECT_FALSE(report.missing.empty());
	}
}	 // namespace xider::tests

#endif	  // __OPENXR__
