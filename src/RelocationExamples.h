#pragma once

namespace RelocationExamples
{
	[[nodiscard]] std::uintptr_t ResolveOneIDForm();

	[[nodiscard]] std::uintptr_t ResolveOGAndSharedNGAEIDs();

	[[nodiscard]] std::uintptr_t ResolveSeparateOGNGAndAEIDs();

	[[nodiscard]] std::uintptr_t ResolveSeparateOGNGAndAEOffsets(
		std::ptrdiff_t a_ogOffset,
		std::ptrdiff_t a_ngOffset,
		std::ptrdiff_t a_aeOffset);

	[[nodiscard]] std::uintptr_t ResolveAutomaticCallsite();
}
