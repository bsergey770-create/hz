# Этап 3 — разбор подсистем, иконки .nlart, C++ SDK

## Шаг 1. Расшифровка карты модулей (analysis/modules.json + string_xrefs.json + iat_usage.json)

### Точные границы зон (по .pdata, всего 16909 функций)
| Зона | RVA-кластеры | Что внутри |
|---|---|---|
| Hook-Engine | 0x20330–0x211cf (trampolin VirtualProtect×2), **0x4285b3–0x4293b0** (потоковые inline-хуки), 0x4b49f0–0x4bc400 (VMT-hook движка), 0x26956b (`[nl_hooks] queue/prediction` — установка на Source2ClientPrediction001), 0x1cf6d0 (`[error] hooks | prediction system ... vtable %p has %d code slots`) | два механизма: классический detour через VirtualProtect+VirtualAlloc (0x428xxx использует OpenThread→SuspendThread→GetThreadContext@0x428790→SetThreadContext@0x4287d1→ResumeThread — паттерн «тихой» правки чужого потока) и копирование/подмена VFT-таблиц интерфейсов Source2 |
| Aimbot/Planner | 0x1e3490 (planner-логирование), 0x29b410 (`state | legit %d ... aimbot %d [%s] act...`), 0x2a1513 (aimbot disabled for group), 0x2a35c8 (quick stop), 0x47b4a0 / 0x47c962 (misprediction-сверка: prediction error / shootpos / lagcomp / hitchance) | rage-ядро: planner::evaluate + texturebug::evaluate + pixelsurf::surf_ticks; 0x47b4a0/0x47c962 — валидация предсказания перед выстрелом |
| Lua-Runtime | 0x9ddb0/0x9e4c0 (lua_owners.txt), 0xff4a0…0x13b9d0 (10 реестров API по ~26 КБ), 0x3211b0 (whitelist ##wl_lua_%s_%d), 0x30bcf0/0x30dca0 (lua_svg/, lua_icon_grid) | LuaJIT 2.1 + 5.1-обвязка, песочница с stage-guard |
| Renderer | 0x1d2ee0 (D3D11 init), 0x3c1950–0x3c51d6 (chams/glow manager, CSceneSystem slot 25 FinishRenderingViews), 0x45e86–0x460c8 ([nl_view] landing bob / CalcView) | ImGui-оверлей + материал-менеджер (*.vmat), HLSL из .rdata |
| Network/License | **0x95270** (единственный пользователь WinHttpOpen/Connect/OpenRequest/SendRequest — весь HTTP-стек в одной функции), 0xd7d70, 0x307870 (neverwin.one + planner-упоминание), 0x323780 (terms/tickets URL), 0x32dc00 (lua.neverwin.one) | лицензионный клиент + загрузчик скриптов |
| Config/UI | 0xf5f60/0xf6940 (дефолты ключей aimbot./targets.*.chams.*), 0x305180/0x30695f/0x307150/0x3076f3 (тексты опций меню), 0x35ef50, 0x4e5650–0x4e5b63 | persistence *.cfg + дерево меню |

### Аимбот — где именно
- **Конфиг-ключи**: `aimbot.enabled` читается в **0xf5f60** (дефолт-таблица) и **0x29b410** (рантайм-резолв состояния); `legitbot.global.aimbot.enabled` / `.triggerbot.enabled` — в **0xf6940**.
- **Логика выбора цели/выстрела**: **0x29b410** (состояние легит/рэйдж, группа, оружие), **0x47b4a0** и **0x47c962** — две ветки сверки предсказания («prediction error / shootpos misprediction / lagcomp misprediction / hitchance misprediction») — это финальный gate перед постановкой IN_ATTACK.
- **Движок плана**: `planner::evaluate(usercmd*, request&, strategy&, sim_tick&, int&)` — код рядом с **0x1e3490** (логи «planner: engine %s flip=%d steer=%d …», «game edgebug fallback»).
- UI-описания опций аимбота (FOV, hitboxes, quick stop, crouch) сидят в описателях меню **0x305180** и **0x30695f** — сами они логику не выполняют.

### Хук CreateMove
- Реальный перехват: **0x26956b** — функция установки на интерфейс `Source2ClientPrediction001` (строка `[nl_hooks] queue %p queue_ret %p entities %p tick +%x signon %p prediction %p…`, site 0x269f10/0x26a010/0x26a0f3). Обёртки команд: кластер **0x5d050–0x5d87f** — 11 маленьких функций-геттеров usercmd, каждая с guard'ом «a command is only valid inside its createmove callback» (API для Lua). Слот get_usercmd/get_usercmd_base/prediction_* регистрируются в **0x3cba6f** (site 0x3cc16f…). Прямой текстовый маркер «createmove» → **0x74460** (site 0x744fd, регистрация имени события).

### CalcView («слот 34»)
- Установщик: **0x4ca18** — одна функция хукает все варианты камер-сервисов: строки `CCSPlayer_CameraServices slot 34` (site 0x4ca4a), `CCSObserver_CameraServices slot 34` (0x4cab1), `CCSObserver_ObserverServices slot 34` (0x4cb15), `CalcView slot 33` (0x4cb79) — т.е. слот 34 = CalcView игрокамеры, слот 33 = резервный вариант.
- Потребитель результата: **0x45e86 / 0x460c8** — `[nl_view] landing bob | blocked before slot 34 (%s CalcView, return %p)` и «slot 34 also runs outside CalcView» — подавление покачивания камеры после приземления.

## Шаг 2. Иконки из .nlart
Секция `.nlart` (RVA 0x115f000, 383 КБ, только чтение) содержит **не atlas из десятков иконок, а ровно 2 векторных документа** (проверено регэкспом `<svg…</svg>` по всей секции; PNG/JPEG-магии — 0):
| Файл | RVA | Размер | viewBox | path-примитивов |
|---|---|---|---|---|
| `analysis/icons/logo_square.svg` | 0x115f000 | 187 526 Б | 160×160 | 2040 |
| `analysis/icons/cracker.svg` | 0x118cc90 | 195 796 Б | 412.41×100 | 2042 |
Оба — результат авто-трассировки растра в кривые (тысячи `<path fill="#RRGGBB" transform="translate(0,0)">`): квадратная эмблема и широкий логотип «cracker». Метка `cracker` лежит сразу вторым блоком (NUL-padding). Lua-скрипты могут добавлять свои SVG через папку-маркер `lua_svg/` (строка 0x30bf9c) — тогда их иконки попадают в сетку `lua_icon_grid`. Извлечены скриптом `step3_extract_icons.py`, манифест — `analysis/icons/manifest.json`.

## Шаг 3. SDK
- `analysis/sdk/neverwin_sdk.hpp` — реконструкция `sdk::usercmd`, `sdk::entity`, `prediction::sim_tick/sim_input`, сигнатуры `planner::evaluate`, `pixelsurf::surf_ticks`, `texturebug::evaluate` (выведены из MSVC-демангла RTTI-лямбд; поля usercmd согласованы со строками отладки «tb local tickbase», «off W+0x40C s (ticks)», «attack1/attack2»).
- `analysis/sdk/lua_runtime_pseudocode.cpp` — псевдокод инициализации Lua-песочницы: загрузка lua_owners.txt, restricted openlibs, ##wl_lua ревизия, stage-guard для engine.simulate/коллбэков, диспетчер createmove.
