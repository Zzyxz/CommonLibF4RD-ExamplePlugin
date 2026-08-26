#pragma once

namespace RelocationExamples
{
	[[nodiscard]] std::uintptr_t ResolvePortableID();

	[[nodiscard]] std::uintptr_t ResolveDualFamilyID();

	[[nodiscard]] std::uintptr_t ResolveExplicitFamilyIDs();

	[[nodiscard]] std::uintptr_t ResolveFamilyOffset(
		std::ptrdiff_t a_ogOffset,
		std::ptrdiff_t a_ngOffset,
		std::ptrdiff_t a_aeOffset);

	[[nodiscard]] std::uintptr_t ResolveAutomaticCallsite();
}
