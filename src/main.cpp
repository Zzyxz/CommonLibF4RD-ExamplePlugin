#include "Plugin.h"

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	// OG, NG, and AE all use this entry point. CommonLibF4RD selects the
	// correct runtime IDs and offsets later, when relocations are requested.
	if (!Plugin::Initialize(a_f4se)) {
		return false;
	}

	return true;
}
