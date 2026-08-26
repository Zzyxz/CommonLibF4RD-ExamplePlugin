#include "RelocationExamples.h"

namespace
{
	// Replace these demonstration IDs with IDs for the functions or objects
	// that the plugin actually uses.

	// REL::ID(AE): one AE ID. NG uses the same ID. OG works only when the
	// Runtime Database contains a verified automatic OG mapping for this ID.
	constexpr REL::ID kOneIDForm{
		2229323  // AE, and also used by NG
	};

	// REL::ID(OG, AE): the first ID is for OG. The second is used by NG and AE.
	constexpr REL::ID kOGAndSharedNGAEIDs{
		1546751,  // OG
		2229323   // NG and AE
	};

	// REL::ID(OG, NG, AE): one explicit ID for every supported game generation.
	constexpr REL::ID kSeparateOGNGAndAEIDs{
		1546751,  // OG
		2229323,  // NG
		2229323   // AE
	};

	// AUTO_CALLSITE searches inside kFunctionContainingHookCall for an
	// instruction that calls kFunctionCalledAtHook.
	constexpr REL::ID kFunctionContainingHookCall{
		1546751,  // OG
		2229323   // NG and AE
	};
	constexpr REL::ID kFunctionCalledAtHook{
		881215,  // OG
		2231148  // NG and AE
	};
}

std::uintptr_t RelocationExamples::ResolveOneIDForm()
{
	REL::Relocation<std::uintptr_t> function{
		kOneIDForm
	};
	return function.address();
}

std::uintptr_t RelocationExamples::ResolveOGAndSharedNGAEIDs()
{
	REL::Relocation<std::uintptr_t> function{
		kOGAndSharedNGAEIDs
	};
	return function.address();
}

std::uintptr_t RelocationExamples::ResolveSeparateOGNGAndAEIDs()
{
	REL::Relocation<std::uintptr_t> function{
		kSeparateOGNGAndAEIDs
	};
	return function.address();
}

std::uintptr_t RelocationExamples::ResolveSeparateOGNGAndAEOffsets(
	std::ptrdiff_t a_ogOffset,
	std::ptrdiff_t a_ngOffset,
	std::ptrdiff_t a_aeOffset)
{
	REL::Relocation<std::uintptr_t> hookSite{
		kSeparateOGNGAndAEIDs,
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
		kFunctionContainingHookCall,
		REL::VariantOffset{
			REL::AUTO_CALLSITE(kFunctionCalledAtHook)
		}
	};
	return hookSite.address();
}
