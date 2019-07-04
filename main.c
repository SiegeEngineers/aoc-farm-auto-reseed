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

static int32_t* allow_command_input = (int32_t*)0x679B10;

// Offset in the player structure that stores an unused byte where only the
// lower bit is filled (always with 1). We can store the autoqueue state in the
// second bit.
static const size_t FARM_AUTOQUEUE_SETTING_OFFSET = 0x1E70;
static const size_t FARM_AUTOQUEUE_MASK = 0x2;

static uint8_t is_autoqueue_enabled(void* player) {
  uint8_t bits = *(uint8_t*)((size_t)player + FARM_AUTOQUEUE_SETTING_OFFSET);
  return bits & FARM_AUTOQUEUE_MASK;
}
static void toggle_autoqueue_flag(void* player) {
  int32_t player_id = *(int32_t*)((size_t)player + 0x9C);

  uint8_t current = *(uint8_t*)((size_t)player + FARM_AUTOQUEUE_SETTING_OFFSET);
  if (current & FARM_AUTOQUEUE_MASK)
    current &= ~FARM_AUTOQUEUE_MASK;
  else
    current |= FARM_AUTOQUEUE_MASK;

  dbg_print("setting autoqueue for %d to %d\n", player_id, !current);

  *(uint8_t*)((size_t)player + FARM_AUTOQUEUE_SETTING_OFFSET) = current;
}

// Add one to the farm queue.

typedef char __thiscall (*fn_queue_farms)(void*, int32_t);
void __thiscall enqueue_farm(size_t player) {
  fn_queue_farms queue_farms = (fn_queue_farms)0x45E4F0;

  dbg_print("enqueue farm for %d\n", *(int32_t*)(player + 0x9C));

  queue_farms((void*)player, 1);
}

// When a farm expired, refill the farm queue if it's empty.

typedef char __thiscall (*fn_auto_rebuild_farm)(void*);
char __thiscall rebuild_farm_hook(void* ptr) {
  fn_auto_rebuild_farm original = (fn_auto_rebuild_farm)0x603D40;

  size_t obj = *(size_t*)((size_t)ptr + 0x8);
  size_t player = *(size_t*)(obj + 0xC);
  int32_t farm_queue_count = *(int32_t*)(player + 0x1EAC);
  uint8_t autoqueue_enabled = is_autoqueue_enabled((void*)player);

  dbg_print("farm_queue_count = %d, should autoqueue = %d\n", farm_queue_count,
            autoqueue_enabled);

  if (farm_queue_count <= 0 && autoqueue_enabled) {
    enqueue_farm(player);
  }

  return original(ptr);
}

// Toggle the automatic queueing setting for the current player.

typedef void* __thiscall (*fn_get_player)(void*);
typedef char* __stdcall (*fn_get_string)(int32_t);
typedef void __thiscall (*fn_add_farm)(void*, int32_t, int32_t);
typedef void __stdcall (*fn_display_message)(char*, int32_t);
void __thiscall toggle_farm_reseed() {
  dbg_print("toggle_farm_reseed()\n");
  fn_add_farm aoc_add_farm = (fn_add_farm)0x46A6F0;
  fn_display_message aoc_display_message = (fn_display_message)0x51CD30;
  fn_get_player aoc_get_player = (fn_get_player)0x5E7560;
  fn_get_string aoc_get_string = (fn_get_string)0x562CE0;

  void* world = *(void**)(*(size_t*)0x7912A0 + 0x424);
  void* player = aoc_get_player(*(void**)0x7912A0);
  // RIP if this happens...
  if (player == NULL)
    return;

  char message[260];
  // Reseed Farm: On|Off
  // Done kinda clumsily because aoc_get_string reuses the buffer,
  // and inverted because we haven't flipped the flag yet.
  sprintf(message, "%s: ", aoc_get_string(4146));
  strcat(message, aoc_get_string(is_autoqueue_enabled(player) ? 10752 : 10751));
  dbg_print("%s\n", message);
  aoc_display_message(message, 1); // 1 = silent

  int32_t player_id = *(int32_t*)((size_t)player + 0x9C);
  void* commander = *(void**)((size_t)world + 0x68);
  // Add 0 farms to indicate toggling automatic reseed queueing,
  // this is meaningless to plain AoC.
  aoc_add_farm(commander, player_id, 0);
}

// Handle a player changing their automatic queueing setting.

int32_t __thiscall add_to_player_queue(void* player, int32_t count) {
  dbg_print("add_to_player_queue()\n");
  fn_queue_farms queue_farms = (fn_queue_farms)0x45E4F0;

  if (count == 0) {
    toggle_autoqueue_flag(player);

#ifndef NDEBUG
    fn_display_message aoc_display_message = (fn_display_message)0x51CD30;
    fn_get_string aoc_get_string = (fn_get_string)0x562CE0;

    char message[260];
    // Reseed Farm: On|Off
    sprintf(message, "<%s> DEBUG %s: ", *(char**)((size_t)player + 0x98),
            aoc_get_string(4146));
    strcat(message,
           aoc_get_string(is_autoqueue_enabled(player) ? 10751 : 10752));
    aoc_display_message(message, 1); // 1 = silent
#endif

    return 1; // Success!
  }

  return queue_farms(player, count);
}

// Allow right-clicking the farm reseed button.

typedef void __thiscall (*fn_configure_button)(void*, void*, int16_t, int16_t,
                                               int32_t, int32_t, int32_t,
                                               int32_t, int32_t, char*, char*,
                                               char*, int32_t);
void __thiscall configure_button(void* screen, void* a, int16_t button_id,
                                 int16_t c, int32_t d, int32_t e, int32_t f,
                                 int32_t g, int32_t h, char* i, char* j,
                                 char* k, int32_t l) {
  fn_configure_button original = (fn_configure_button)0x520620;
  original(screen, a, button_id, c, d, e, f, g, h, i, j, k, l);

  dbg_print("configure_button(%d)\n", d);
  if (d != 173)
    return;

  void* button = *(void**)((size_t)screen + 0x1080 + 4 * button_id);

  // This is reset by the game before redrawing the buttons list so
  // we don't have to worry about lingering state.
  *(int32_t*)((size_t)button + 0x2D0) = d == 173;
}

// Handle right click action on the farm reseed button.

typedef void __thiscall (*fn_do_button_action)(void*, int32_t, int32_t,
                                               int32_t);
static fn_do_button_action aoc_do_button_action = NULL;
void __thiscall do_button_action(void* screen, int32_t action_in,
                                 int32_t action2_in, int32_t right_btn) {
  dbg_print("do_button_action(%d, %d, %d)\n", action_in, action2_in, right_btn);

  if (action_in == 173 && action2_in == 50 && right_btn == 1) {
    dbg_print("allow_command_input = %d\n", *allow_command_input);
    if (*allow_command_input) {
      toggle_farm_reseed();
    }

    // The right-click handling code actually _unqueues_ a farm reseed in the
    // base game, but you couldn't right-click the button. We can safely
    // override it, but it does mean we need to return early here.
    return;
  }

  aoc_do_button_action(screen, action_in, action2_in, right_btn);
}

void mmm_load(mmm_mod_info* info) {
  info->name = "Automatic farm reseeding";
  info->version = AOC_FARM_AUTO_RESEED_VERSION;
}

void mmm_before_setup(mmm_mod_info* info) {
  dbg_print("init()\n");

  install_callhook((void*)0x467809, (void*)add_to_player_queue);
  install_callhook((void*)0x602FD3, (void*)rebuild_farm_hook);
  install_callhook((void*)0x60305B, (void*)rebuild_farm_hook);
  install_callhook((void*)0x527C59, (void*)configure_button);
  aoc_do_button_action = (fn_do_button_action)(0x51E1C9 + *(int32_t*)0x51E1C5);
  dbg_print("aoc_do_button_action = %p\n", aoc_do_button_action);
  install_callhook((void*)0x51DBC0, (void*)do_button_action);
  install_callhook((void*)0x51E1C4, (void*)do_button_action);
  install_callhook((void*)0x51E209, (void*)do_button_action);
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
