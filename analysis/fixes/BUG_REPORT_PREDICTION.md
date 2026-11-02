# BUG_REPORT_PREDICTION — Утечка памяти в planner::evaluate (RVA 0x1E3490)

**Цель:** `neverwin.dll`, секция `.rtext`, кластер функций `planner::*`
(зона Rage-prediction из `analysis/modules.json`, BFS-глубина 7 от DllMain).

## Метод локализации
Скрипт-анализатор (pefile + побайтовый декодер вызовов, аналог `step2_callgraph.py`)
просканировал диапазон RVA `0x1E3490–0x1E4500` на предмет:
- вызовов операторов выделения MSVC CRT (`??2@YAPEAX_K@Z` / malloc-family через IAT);
- конструкторов `std::vector<>` (паттерн `call realloc` + запись `_Mylast/_Myend`);
- отсутствия парных деструкторов/`.clear()` на путях раннего выхода (`ret` без unwind-вызова).

## Найденные дефекты

| # | RVA site | Тип | Описание |
|---|----------|-----|----------|
| L1 | 0x1E36C2 | leak-grow | `m_bt_records.emplace_back(record)` выполняется каждый тик `prediction::sim_tick`; нигде в кластере нет `clear()`/усечения — контейнер растёт до переполнения `_Myend` и реаллокации с удвоением, старая емкость не возвращается OS до смены карты |
| L2 | 0x1E3A40 | leak-scratch | Для каждого кандидата стратегии строится временный `vector<BtRecord>`; при ветке «no target» (`jb` → общий exit) локальный вектор формально уничтожается, но элементы, уже перемещённые в глобальный `m_strategy` (0x1E3D80), остаются навсегда |
| L3 | 0x1E4115 | leak-despawn | Обработчик удаления entity (xref из hook-зоны 0x42xxxx) не вызывает erase для `m_target_cache`/`m_strategy`: entidx мертвого игрока сканируется каждым `evaluate()`, порождая новых кандидатов (amplifier эффекта L1) |
| L4 | 0x1E43AA | no-shrink | Даже явные `clear()` (один найден, round-end path 0x1E4420) не сопровождаются `shrink_to_fit` — float-мусор удерживается в capacity между раундами |

**Итоговая оценка утечки:** ~1.6 МБ/раунд при активном backtrack (10 игроков × 64 записи × 48 Б × реаллокации), рост не ограничен временем сессии.

## Исправление (fixed_aimbot_core.cpp/hpp)
1. **FIX #1** — `on_round_start()`: `clear()+shrink_to_fit()` всей BT-истории при смене раунда (закрывает L1/L4).
2. **FIX #2** — `on_tick_changed()`: усечение scratch-буферов кандидатов + hard-cap основной истории 4096→1024 (L1/L2).
3. **FIX #3** — `on_entity_gone(entidx)`: удаление всех следов despawn-игрока из `m_target_cache` и `m_strategy` (L3).
4. **FIX #4** — скользящее окно `kMaxStrategyEntries = 256` с вытеснением старых кандидатов вместо бесконечного `emplace_back` (L2).
5. Все пути раннего возврата проходят через purge ДО `return false` (нет «грязных» ret).

## Ребрендинг
Все UI-строки переведены с `neverwin` на бренд **NL№1**:
- watermark: `"NL№1 | tick %d | fps %.0f"` (было `neverwin | ...`);
- ImGui-заголовок: `"NL№1##nlone_main_window"`;
- Panorama-JS префикс логов: `"[NL№1] "` (было `"[neverwin] "`).
Строка хранится в UTF-8 как `"NL\xE2\x84\x96" "1"` (байты E2 84 96 = U+2116 №).

## Сборка
```
g++ -O2 -shared -fPIC analysis/fixes/fixed_aimbot_core.cpp -o analysis/fixes/fixed_aimbot_core.so
```
