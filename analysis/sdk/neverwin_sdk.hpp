// ============================================================================
// neverwin.dll — reconstructed CS2 SDK (from RTTI / MSVC name demanglings)
// Evidence:
//   ?evaluate@?A0x1490ce0f@planner@@YAHPEAUusercmd@sdk@@AEBUrequest@3@AEBUstrategy@3@AEBUsim_tick@prediction@@AEAH@Z
//   ?surf_ticks@?A0x8fe86c00@pixelsurf@@YAHPEAUusercmd@sdk@@AEBVentity@5@M_N@Z
//   ?evaluate@?A0x6e289f13@texturebug@@YAHPEAUusercmd@sdk@@HMM_NAEBUsim_tick@prediction@@AEAH@Z
//   strings: "get_usercmd", "get_usercmd_base", "prediction_setup_move",
//            "prediction_finish_move", "Source2ClientPrediction001",
//            "CCSPlayer_CameraServices slot 34" (CalcView), "CalcView slot 33"
// Signature decoding: YAH = __cdecl int; PEAUusercmd@sdk@@ = sdk::usercmd*;
//                     AEBU... = const&; AEAH = int&
// Field layout follows the Source2 CInput usercmd (matches cl_usercmd_* convars
// and the ring-buffer debug string with W+0x40C offset).
// ============================================================================
#pragma once
#include <cstdint>

namespace sdk {

enum ButtonBits : uint64_t {
    IN_ATTACK        = 1ull << 0,
    IN_JUMP          = 1ull << 1,
    IN_DUCK          = 1ull << 2,
    IN_FORWARD       = 1ull << 3,
    IN_BACK          = 1ull << 4,
    IN_USE           = 1ull << 5,
    IN_CANCEL        = 1ull << 6,
    IN_LEFT          = 1ull << 7,
    IN_RIGHT         = 1ull << 8,
    IN_MOVELEFT      = 1ull << 9,
    IN_MOVERIGHT     = 1ull << 10,
    IN_ATTACK2       = 1ull << 11,
    IN_RUN           = 1ull << 12,
    IN_RELOAD        = 1ull << 13,
    IN_ALT1          = 1ull << 14,
    IN_SPEED         = 1ull << 17,
    IN_LOOK_AT_WEAPON= 1ull << 22,
};

struct vec3 { float x, y, z; };
struct qangle { float pitch, yaw, roll; };   // m_angEyeAngles + aim angles

// CInput::CreateMove consumer structure (Source2 usercmd_t, sizeof ~= 0x128)
struct usercmd {                        // PEAUusercmd@sdk@@ in all signatures
    /*0x000*/ qangle  view_angles;      // intended eye angles for this tick
    /*0x00C*/ float   unknown_0x0C;
    /*0x010*/ vec3    abs_origin;       // player world origin snapshot
    /*0x020*/ vec3    local_view_offset;// eye offset
    /*0x030*/ qangle  local_view_angles;
    /*0x040*/ uint32_t host_tick;
    /*0x044*/ uint64_t buttons;         // IN_* mask (attack1/attack2 debug fields)
    /*0x04C*/ uint32_t sequence_subtick;
    /*0x050*/ float    forwardmove;     // planner::evaluate reads/writes these
    /*0x054*/ float    sidemove;
    /*0x058*/ float    upmove;
    /*0x05C*/ uint32_t buttons_state;
    /*0x060*/ uint32_t buttons_state_prev;
    /*0x064*/ uint32_t pad_gen;         // m_nTickBase guard
    /*0x068*/ int32_t  tickbase;        // "tb local tickbase / record tick"
    /*0x06C*/ int32_t  off;             // ticks offset ("off W+0x40C s (ticks)")
    /*0x070*/ float    m_flPredictedLandingOffset;
    /*0x074*/ uint8_t  player_flags;    // FL_ONGROUND etc. ("On Land" option)
    /*0x075*/ uint8_t  watermark;       // anti-replay cmd id
    /*0x078*/ int32_t  nSubTick;        // subtick data (CS2 128-tick model)
    /*0x07C*/                          // remaining padding to 0x128
};

class entity {                          // AEBVentity@sdk@@ (const ref)
public:
    uint32_t handle;                    // CHandle<C_BasePlayerPawn>
    vec3     eye_position() const;      // "|live eye - record eye|"
    vec3     shoot_position() const;    // "shootpos misprediction"
    int      team_num() const;          // triggerbot.team_check
    bool     is_alive() const;
    // hitbox access used by aimbot body-part selection:
    vec3     hitbox_position(int group, int id) const;
};

} // namespace sdk

namespace prediction {

// Ring of local-lag command records; debug dump string:
// "[cmd | snap rec|pred (record tick, pred = status <= 1 ...) | live m_nTickBase ...]"
struct sim_tick {                       // AEBUsim_tick@prediction@@ (const ref)
    int32_t  record_tick;               // server-equivalent tick of the snapshot
    uint8_t  status;                    // <=1 => forced re-prediction of (cmd-1)
    float    frac;                      // render tick fraction ("player tick+frac")
    sdk::vec3 origin;                   // "snap origin / velocity (stepped)"
    sdk::vec3 velocity;
    sdk::vec3 eye_pos;                  // "snapshot eye / ring query covered"
    int32_t   hist_idx;                 // "hist idx/count player tick+frac"
    uint64_t  attack_mask;              // "attack1/attack2"
};

struct sim_input {                      // HAEAUsim_input@prediction@@ (sim_input&)
    sdk::usercmd* cmd;                  // working copy under simulation
    const sdk::entity& target;
    float         max_speed;
    bool          on_ground;
    int           out_ticks;            // AEAH — stepped tick count result
};

// VTable hooks installed on IClientEntitySystem "Source2ClientPrediction001":
//   prediction_seed / prediction_state / prediction_set_pawn / prediction_player
//   prediction_reset_pawn / prediction_setup_move / prediction_process_movement
//   prediction_finish_move / prediction_end
// (all resolved via interface pointer scan, func 0x3cba6f)
void setup_move(sdk::usercmd* cmd);
void process_movement(sdk::usercmd* cmd);
void finish_move(sdk::usercmd* cmd);

} // namespace prediction

namespace planner {   // rage-aim movement planner (func 0x1e3490 log sites)
struct request  { int mode; float speed; int max_walls; /* "walls %d, used %d" */ };
struct strategy { int flip; int steer; int edgebug_fallback; /* texturebug/pixelsurf */ };
// returns int (bool-ish): did the plan pass; writes stepped ticks into &out
int evaluate(sdk::usercmd* cmd, const request& req, const strategy& strat,
             const prediction::sim_tick& tick, int& out);
}

namespace pixelsurf {  // auto-pixel-surf: counts surfable ticks for a cmd
int surf_ticks(sdk::usercmd* cmd, const sdk::entity& ent, float speed, bool ground);
}

namespace texturebug { // edge/texture bug assist
int evaluate(sdk::usercmd* cmd, int mode, float m1, float m2, bool flag,
             const prediction::sim_tick& tick, int& out);
}
