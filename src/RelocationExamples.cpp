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

std::uintptr_t RelocationExamples::ResolveAutomaticCallsiteWithKnownOffsets()
{
	// Problem: another plugin may already have replaced this call with its own
	// hook, for example with write_call. The call then no longer goes directly
	// to Actor::DoHitMe, so the automatic search in ResolveAutomaticCallsite()
	// finds no match and resolution fails.
	//
	// or_offset adds the offset of the call for exact game versions on which it
	// has been checked. On a listed version, CommonLibF4RD reads the instruction
	// at that offset first and uses it if it
	//   - still calls Actor::DoHitMe, or
	//   - calls code outside Fallout4.exe, which means another plugin has
	//     already redirected this call.
	// In the second case, write_call returns the other plugin's hook as the
	// original function, so both hooks run one after the other.
	//
	// If the check fails, or the running version is not listed, the normal
	// automatic search runs exactly as in ResolveAutomaticCallsite(). A game
	// update that moves the call is therefore never patched at a stale offset.
	//
	// Each or_offset accepts up to four versions. The offset is relative to the
	// caller, like every other VariantOffset value.
	REL::Relocation<std::uintptr_t> hookSite{
		kFunctionThatCallsActorDoHitMe,
		REL::VariantOffset{
			// OG slot
			REL::AUTO_CALLSITE(kActorDoHitMe)
				.or_offset(0x921, REL::Version{ 1, 10, 163, 0 }),
			// NG slot
			REL::AUTO_CALLSITE(kActorDoHitMe)
				.or_offset(0x8F7, REL::Version{ 1, 10, 984, 0 }),
			// AE slot: list only the versions on which the offset was checked.
			// Other 1.11.x versions use the automatic search.
			REL::AUTO_CALLSITE(kActorDoHitMe)
				.or_offset(0x8F7, REL::Version{ 1, 11, 221, 0 }, REL::Version{ 1, 11, 240, 0 })
		}
	};
	return hookSite.address();
}

std::uintptr_t RelocationExamples::TryResolveAutomaticCallsite()
{
	// REL::Relocation stops the game with an error message when an ID or a
	// callsite cannot be resolved. That is the right behavior for anything the
	// plugin cannot work without.
	//
	// For an optional feature, REL::try_resolve_callsite returns an empty result
	// instead. The plugin can log the reason, skip only this one hook, and keep
	// the rest of the plugin running. It accepts the same VariantOffset values,
	// including AUTO_CALLSITE with or_offset.
	const auto lookup = REL::try_resolve_callsite(
		kFunctionThatCallsActorDoHitMe,
		REL::VariantOffset{
			REL::AUTO_CALLSITE(kActorDoHitMe)
		});

	if (!lookup) {
		// status is the short reason, for example callsite_not_found.
		// note adds details when available, for example why a known offset
		// from or_offset was rejected.
		logger::warn(
			"optional Actor::DoHitMe hook disabled: {} {}",
			REL::id_resolve_status_text(lookup.resolution.status),
			lookup.resolution.note);
		return 0;
	}

	// note can also be set on success, for example when a known offset was used
	// because another plugin had already hooked the call.
	if (!lookup.resolution.note.empty()) {
		logger::info("Actor::DoHitMe hook site: {}", lookup.resolution.note);
	}
	return *lookup.address;
}
