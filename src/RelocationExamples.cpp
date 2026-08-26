#include "RelocationExamples.h"

namespace
{
	// These IDs demonstrate the API and must be replaced with symbols required
	// by the plugin before the corresponding example function is called.
	constexpr REL::ID kPortableID{ 2229323 };
	constexpr REL::ID kDualFamilyID{ 1546751, 2229323 };
	constexpr REL::ID kExplicitFamilyIDs{ 1546751, 2229323, 2229323 };

	constexpr REL::ID kCallOwner{ 1546751, 2229323 };
	constexpr REL::ID kCallTarget{ 881215, 2231148 };
}

std::uintptr_t RelocationExamples::ResolvePortableID()
{
	REL::Relocation<std::uintptr_t> function{
		kPortableID
	};
	return function.address();
}

std::uintptr_t RelocationExamples::ResolveDualFamilyID()
{
	REL::Relocation<std::uintptr_t> function{
		kDualFamilyID
	};
	return function.address();
}

std::uintptr_t RelocationExamples::ResolveExplicitFamilyIDs()
{
	REL::Relocation<std::uintptr_t> function{
		kExplicitFamilyIDs
	};
	return function.address();
}

std::uintptr_t RelocationExamples::ResolveFamilyOffset(
	std::ptrdiff_t a_ogOffset,
	std::ptrdiff_t a_ngOffset,
	std::ptrdiff_t a_aeOffset)
{
	REL::Relocation<std::uintptr_t> hookSite{
		kExplicitFamilyIDs,
		REL::VariantOffset{
			a_ogOffset,
			a_ngOffset,
			a_aeOffset
		}
	};
	return hookSite.address();
}

std::uintptr_t RelocationExamples::ResolveAutomaticCallsite()
{
	REL::Relocation<std::uintptr_t> hookSite{
		kCallOwner,
		REL::VariantOffset{
			REL::AUTO_CALLSITE(kCallTarget)
		}
	};
	return hookSite.address();
}
