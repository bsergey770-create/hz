// ============================================================================
// Lua sandbox initialization — pseudocode reconstruction
// Evidence (neverwin.dll static analysis):
//   RTTI:  ?...@detail@lua_runtime@@YAHPEAUlua_State@@@Z  (func 0xb27e2512 lambda)
//   demangled namespace method: lua_runtime::engine_simulate
//   strings: "C:\neverwin\lua_owners.txt", "##wl_lua_%s_%d", "lua_icon_grid",
//            "lua_svg/", "lua_output", "engine.simulate works only inside createmove",
//            "%s works only in createmove, frame_stage and game event callbacks"
//   Lua-Runtime zone functions (modules.json): 0x9ddb0, 0x9e4c0 (owner check),
//     0xff4a0..0x13b9d0 (per-feature API registration tables), 0x32dc00 (lua.neverwin.one),
//     0x3211b0 (whitelist name fmt "##wl_lua_%s_%d")
//   Embedded interpreter: LuaJIT 2.1.1788856981 + Lua 5.1 compat layer.
// ============================================================================

// ---- Globals captured from the binary -------------------------------------
struct lua_State;                       // LuaJIT state, embedded in DLL
namespace sdk { struct usercmd; }
namespace prediction { struct sim_tick; }

// ---------------------------------------------------------------------------
// 1) Owner allow-list gate (reconstructed from funcs 0x9ddb0 / 0x9e4c0)
// ---------------------------------------------------------------------------
static std::vector<std::string> g_owner_ids;    // loaded once at startup

int lua_runtime__load_owners()                   // ~RVA 0x9ddb0
{
    FILE* f = fopen("C:\\neverwin\\lua_owners.txt", "r");   // string @0x9de02
    if (!f) return -1;                           // no owners file => deny all scripts
    char line[260];
    while (fgets(line, sizeof line, f))
        g_owner_ids.push_back(trim(line));       // one SteamID/account-id per line
    fclose(f);
    return (int)g_owner_ids.size();
}

bool lua_runtime__is_owner_allowed(uint64_t account_id)   // ~RVA 0x9e4c0
{
    // called before ANY user script is mounted; error path writes to "lua_output"
    return std::find(g_owner_ids.begin(), g_owner_ids.end(), to_string(account_id))
           != g_owner_ids.end();
}

// ---------------------------------------------------------------------------
// 2) Sandbox creation & library lockdown
// ---------------------------------------------------------------------------
lua_State* lua_runtime__create_sandbox(script_ctx* sc)
{
    lua_State* L = luaL_newstate();              // LuaJIT: lj_state.c
    luaL_openlibs_restricted(L);                 // NOT full openlibs:
        // keep: base (sans dofile/loadstring/system), table, string, math
        // drop: io, os (except os.clock/time), package, debug
    // whitelist marker for this instance: "##wl_lua_%s_%d" (fmt site 0x3214cf)
    //   %s = script name, %d = revision -> stored in registry for hot-reload diff
    lua_pushlstring(L, wl_key(sc->name, sc->rev), ...);
    lua_setfield(L, LUA_REGISTRYINDEX, "##wl_lua");

    // --- inject neverwin API namespaces (registration funcs 0xff4a0..0x13b9d0)
    lua_register_entity_api(L);                  // entity/players iteration
    lua_register_render_api(L);                  // draw primitives, ImDrawList bridge
    lua_register_config_api(L);                  // triggerbot.enabled/team_check..., 
                                                 // grenade_prediction.*, watermark.*
    lua_register_events_api(L);                  // createmove/frame_stage/game events
    // svg/icon helpers: menu can render user assets ("lua_svg/" @0x30bf9c,
    // "lua_icon_grid" @0x30eded) — resolved against .nlart SVG atlas
    return L;
}

// ---------------------------------------------------------------------------
// 3) Stage dispatch — the guard behind the two "works only ..." errors
// ---------------------------------------------------------------------------
enum class stage : int { NONE, CREATEMOVE, FRAME_STAGE, GAME_EVENT };
thread_local stage g_current_stage;              // set by hook trampolines
thread_local sdk::usercmd* g_current_cmd;        // valid only in CREATEMOVE

int lua_runtime__engine_simulate(lua_State* L)   // C function exposed to scripts
{
    if (g_current_stage != stage::CREATEMOVE || !g_current_cmd) {
        // string @0xa5326: "engine.simulate works only inside createmove"
        return luaL_error(L, "engine.simulate works only inside createmove");
    }
    // Re-run the prediction pipeline on a CLONE of the live cmd:
    sdk::usercmd probe = *g_current_cmd;         // copy: view_angles, tickbase, off...
    prediction::sim_tick tick = prediction_ring_query(g_current_cmd->off);
    int stepped = 0;
    // planner::evaluate(usercmd*, request&, strategy&, sim_tick&, int& out)
    bool ok = planner::evaluate(&probe, cur_request(), cur_strategy(), tick, stepped);
    // push resulting simulated position/velocity back onto the Lua stack
    lua_newtable(L);  push_vec(L, probe.abs_origin);  push_int(L, stepped);
    lua_pushboolean(L, ok);
    return 2;                                    // (simulated_state, success)
}

void lua_runtime__fire(const char* evt, lua_State* L)
{
    // wrappers registered at 0x60990/0x60c50/0x60fc0/0xa64a0/0xa6890/0xa6a10/0xa7060
    // all share the guard string:
    //   "%s works only in createmove, frame_stage and game event callbacks"
    if (g_current_stage == stage::NONE)
        luaL_error(L, "%s works only in createmove, frame_stage and game event callbacks", evt);
    lua_call_guarded(L, evt);                    // pcall with output redirect to
                                                 // config key "lua_output" panel
}

// ---------------------------------------------------------------------------
// 4) Per-frame lifecycle (called from DllMain-initialized subsystem, BFS root)
// ---------------------------------------------------------------------------
void lua_runtime__tick(sdk::usercmd* cmd)        // invoked by CreateMove trampoline
{
    g_current_stage = stage::CREATEMOVE; g_current_cmd = cmd;
    for (auto& sc : g_scripts)                   // hot-reload checked via ##wl_lua key
        if (sc.has_handler("createmove")) fire("createmove", sc.L);
    g_current_stage = stage::NONE; g_current_cmd = nullptr;
}
