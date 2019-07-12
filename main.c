#include "farm_auto_reseed.h"
#include "hook.h"
#include <mmmod.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#define AOC_FARM_AUTO_RESEED_VERSION "1.0.0-alpha.0"

#ifdef NDEBUG
#define dbg_print(...)
#else
#define dbg_print(...) printf("[aoc-farm-auto-reseed] " __VA_ARGS__)
#endif

void mmm_load(mmm_mod_info* info) {
  info->name = "Automatic farm reseeding";
  info->version = AOC_FARM_AUTO_RESEED_VERSION;
}

void mmm_before_setup(mmm_mod_info* info) {
  dbg_print("mmm_before_setup()\n");

  aoc_farm_auto_reseed_setup();
}

void mmm_unload(mmm_mod_info* info) { dbg_print("mmm_unload()\n"); }

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, void* _) {
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(dll);
    break;
  }
  return 1;
}
