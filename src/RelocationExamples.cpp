#include "RelocationExamples.h"

namespace
{
	// This real example uses Actor::DoHitMe. The IDs are Runtime Database IDs,
	// not addresses and not Fallout 4 version numbers.
	constexpr REL::ID kActorDoHitMe{
		881215,  // Actor::DoHitMe on OG
		2231148  // Actor::DoHitMe on NG and AE
	};

	// This is a different function. It contains the particular call to
	// Actor::DoHitMe that the example wants to replace.
	constexpr REL::ID kFunctionThatCallsActorDoHitMe{
		1546751,  // surrounding caller function on OG
		2229323   // the same logical caller function on NG and AE
	};
}

std::uintptr_t RelocationExamples::ResolveOneIDForm()
{
	// REL::ID(AE)
	// NG and AE use 2231148 directly. On OG, CommonLibF4RD tries to map this
	// ID through a verified automatic bridge. It fails instead of guessing
	// when no safe OG mapping exists.
	REL::Relocation<std::uintptr_t> actorDoHitMe{
		REL::ID{ 2231148 }
	};
	return actorDoHitMe.address();
}

std::uintptr_t RelocationExamples::ResolveOGAndSharedNGAEIDs()
{
	// REL::ID(OG, AE)
	// OG uses 881215. NG and AE both use 2231148.
	REL::Relocation<std::uintptr_t> actorDoHitMe{
		REL::ID{ 881215, 2231148 }
	};
	return actorDoHitMe.address();
}

std::uintptr_t RelocationExamples::ResolveSeparateOGNGAndAEIDs()
{
	// REL::ID(OG, NG, AE)
	// This form supplies every runtime-family ID explicitly.
	REL::Relocation<std::uintptr_t> actorDoHitMe{
		REL::ID{ 881215, 2231148, 2231148 }
	};
	return actorDoHitMe.address();
}

std::uintptr_t RelocationExamples::ResolveSeparateOGNGAndAEOffsets(
	std::ptrdiff_t a_ogOffset,
	std::ptrdiff_t a_ngOffset,
	std::ptrdiff_t a_aeOffset)
{
	// VariantOffset uses the same runtime order as REL::ID.
	REL::Relocation<std::uintptr_t> hookSite{
		kFunctionThatCallsActorDoHitMe,
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
	// Goal: replace one specific call to Actor::DoHitMe, not the function
	// entry and not every Actor::DoHitMe call in the game.
	//
	// CommonLibF4RD resolves both functions, searches only inside the caller,
	// and returns the unique direct call whose destination is Actor::DoHitMe.
	REL::Relocation<std::uintptr_t> hookSite{
		kFunctionThatCallsActorDoHitMe,
		REL::VariantOffset{
			REL::AUTO_CALLSITE(kActorDoHitMe)
		}
	};
	return hookSite.address();
}
