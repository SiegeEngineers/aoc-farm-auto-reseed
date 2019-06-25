#include "hook.h"
#include <mmmod.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#define AOC_FARM_AUTO_RESEED_VERSION "0.0.0-alpha.0"

#ifdef NDEBUG
#define dbg_print(...)
#else
#define dbg_print(...) printf("[aoc-farm-auto-reseed] " __VA_ARGS__)
#endif

#ifdef BREAKSYNC

// If we're allowed to break older game versions attempting to play back this
// rec, we can use the simple reseeding implementation

typedef char __thiscall (*fn_queue_farms)(void*, int32_t);
void __thiscall enqueue_farm(size_t player) {
  fn_queue_farms queue_farms = (fn_queue_farms)0x45E4F0;

  dbg_print("enqueue farm for %d\n", *(int32_t*)(player + 0x9C));

  queue_farms((void*)player, 1);
}

#else

// If we're not allowed to break sync, we need to send some actions so that
// older game versions will do the right thing when playing this rec back. This
// part is not yet done :D
//
// perhaps just one, only a rebuild command?
// need to make sure that the player has enough resources, and only send the
// expired notification if not since MP actions are queued that is a little
// tricky; probably need to move the expired notification to a later time, when
// the action is actually run

typedef char __thiscall (*fn_queue_farms)(void*, int32_t, int32_t);
void __thiscall enqueue_farm(size_t player) {
  fn_queue_farms queue_farms = (fn_queue_farms)0x46A6F0;

  void* game = (void*)0x7912A0;
  size_t plr = get_player(game);
  int32_t player_id = *(int32_t*)(plr + 0x9C);

  dbg_print("enqueue farm for %d\n", player_id);

  size_t world = *(size_t*)(game + 0x424);
  queue_farms(*(void**)(world + 0x68), player_id, 1);
}

#endif

typedef char __thiscall (*fn_auto_rebuild_farm)(void*);
char __thiscall rebuild_farm_hook(void* ptr) {
  fn_auto_rebuild_farm original = (fn_auto_rebuild_farm)0x603D40;

  size_t obj = *(size_t*)((size_t)ptr + 0x8);
  size_t plr = *(size_t*)(obj + 0xC);
  int32_t farm_queue_count = *(int32_t*)(plr + 0x1EAC);
  dbg_print("farm_queue_count = %d\n", farm_queue_count);

  if (farm_queue_count <= 0) {
    enqueue_farm(plr);
  }

  return original(ptr);
}

void mmm_load(mmm_mod_info* info) {
  info->name = "Automatic farm reseeding";
  info->version = AOC_FARM_AUTO_RESEED_VERSION;
}

void mmm_before_setup(mmm_mod_info* info) {
  dbg_print("init()\n");

  install_callhook((void*)0x602FD3, (void*)rebuild_farm_hook);
  install_callhook((void*)0x60305B, (void*)rebuild_farm_hook);
}

void mmm_unload(mmm_mod_info* info) { dbg_print("deinit()\n"); }

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, void* _) {
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(dll);
    break;
  }
  return 1;
}
