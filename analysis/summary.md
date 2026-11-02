# Static analysis summary — neverwin.dll

SHA-256: fa3a986c46fb6a766c8c1051cf38e8255571211fc5572b8b898f4b3759c0667d
Size: 13,896,192 bytes

## PE header
- Machine: 0x8664 (AMD64), DLL, image base 0x180000000, SizeOfImage 0x11bd000 (~18.3 MB)
- DllCharacteristics: 0x160 → DYNAMIC_BASE|NX_COMPAT|GUARD_CF|TERMINAL_SERVER_AWARE
- Entry point RVA 0x4ed880 = classic MSVC CRT dllEntryPoint (checks reason==1, tail-jumps to real DllMain at 0x4ed740)
- Linker version 14.4x (Visual Studio 2022), TimeDateStamp 0x6add950a

## Sections
.rtext/.pdata/.fptable custom names + .text/.rdata/.data/.bind/.idata/.tls — renamed section layout is itself an anti-analysis fingerprint.

## Imports (statically bound)
d3d11.dll (D3D11CreateDeviceAndSwapChain), D3DCOMPILER_47 (D3DCompile), USER32 (45: windowing/input/clipboard),
KERNEL32 (181: full CRT-support set + Rtl* unwind, Get/SetThreadContext, OpenThread, Suspend/ResumeThread,
CreateToolhelp32Snapshot/Thread32First/Next — thread enumeration typical of manual-map loaders & anti-debug),
ADVAPI32 (registry), WINHTTP (full client stack — network/license calls resolved via WinHttpCrackUrl), SHELL32, ole32, IMM32, GDI32.
Most other APIs are resolved dynamically via LoadLibrary/GetProcAddress (all present).

## Exports
177 exports — ONLY C/CRT stdio printf-family (_snprintf, vfprintf_s, swscanf…). This is the signature of a
**manual-mapped DLL**: it pretends to be a CRT shim so that any loader can bind printf functions; no real feature API exported.

## Embedded resources / strings (25k strings extracted)
- Panorama JS blocks with console prefix '[neverwin]' (agent preview, ESP preview, hud removals, lobby 'cancel match'/'auto accept' hooks)
- HLSL shader sources ('neverwin_tint', star layers, skybox EvaluateSky hooks) — CS2 scene-system overrides
- LuaJIT 2.1.1788856981 / Lua 5.1 banner + lua_runtime::engine_simulate symbol → embedded Lua scripting engine
- ImGui (ocornut/imgui FAQ URL) for the overlay UI
- Config tree keys: watermark.*, chams, glow, triggerbot exits list ('rage: no_record…'), aimbot activation modes, skin settings
- Paths: C:\neverwin\{configs,langs,scripts,particles,images,fonts}, lua_owners.txt
- URLs: https://neverwin.one/help/terms/, https://neverwin.one/tickets/, https://lua.neverwin.one
- MSVC RTTI demangles show internal namespaces: prediction::sim_tick, planner::request/strategy, sdk::usercmd/entity,
  lua_runtime::detail, skybox::compile_result, grenade_fire::simulation, pixelsurf, texturebug, econ::item, config::skin_settings, menu

## Anti-analysis observed
- DllMain PROCESS_ATTACH handler (RVA 0x4edc50): builds a checksum from GetTickCount64+GetProcessHeap(?) values, XOR-folds it,
  stores magic 0x2b992ddfa232/negation pair in two globals — self-verification/integrity sentinel pattern.
- TLS directory present (StartVA 0x1808b23c0, callbacks array VA 0x180c9f468 in .fptable) — TLS-callback based init before DllMain.
- .fptable (function-pointer table for CFG) + GUARD_CF flag set.
- Section renaming (.rtext/.fptable/.bind/.idata/.tls) and CRT-export disguise.
- Thread-enumeration/context imports (OpenThread/GetThreadContext/SetThreadContext/SuspendThread + Toolhelp32) consistent with
  attach-time patching of the host process threads (typical for injected CS2 cheats hooking client.dll).

