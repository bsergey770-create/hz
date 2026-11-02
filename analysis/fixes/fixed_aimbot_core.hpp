// ============================================================================
// fixed_aimbot_core.hpp — NL№1 (rebrand из neverwin)
// Исправленное ядро rage-предикшена: planner::evaluate + детерминированная
// очистка Backtrack Records / strategy-векторов.
// Референс оригинала: neverwin.dll, RVA кластера 0x1E3490 (planner::evaluate),
// SDK-неймспейсы sdk::usercmd, prediction::sim_tick (см. analysis/sdk).
// ============================================================================
#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>

namespace nlone { // бренд NL№1 (ex-neverwin)

// ---------------------------------------------------------------------------
// Константы бренда. ВАЖНО: все UI-строки используют ТОЛЬКО эти определения,
// чтобы замена "neverwin" -> "NL№1" была централизованной.
// UTF-8 байты для "№": E2 84 96
// ---------------------------------------------------------------------------
inline constexpr const char* BRAND_NAME        = "NL\xE2\x84\x96" "1"; // UTF-8 "NL№1"
inline constexpr const char* BRAND_NAME_SP     = "NL \xE2\x84\x96 1"; // читаемый вариант с пробелами
inline constexpr const char* WATERMARK_FORMAT  = "%s | tick %d | fps %.0f";
inline constexpr const char* MENU_TITLE_FORMAT = "%s##nlone_main_window";
inline constexpr const char* PANORAMA_PREFIX   = "[NL \xE2\x84\x96 1] "; // было "[neverwin] "

// ---------------------------------------------------------------------------
// Базовые типы (согласованы с analysis/sdk/neverwin_sdk.hpp)
// ---------------------------------------------------------------------------
struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;
};

struct QAngle {
    float pitch = 0.f, yaw = 0.f, roll = 0.f;
};

// Запись истории игрока (backtrack record)
struct BtRecord {
    int32_t tick            = 0;
    float   simulation_time = 0.f;
    Vec3    origin{};
    Vec3    eye_pos{};
    float   duck_amount     = 0.f;
    bool    alive           = false;
};

// Кандидат стратегии мувера (аллоцировался в evaluate и НЕ очищался в оригинале)
struct StrategyEntry {
    int32_t target_entidx          = -1;
    float   score                  = 0.f;
    QAngle  aim_angle{};
    bool    backtrack_used         = false;
    std::vector<BtRecord> history_scratch; // временный буфер — источник утечек
};

// Входные данные тика предсказания
struct SimInput {
    uint32_t tickbase      = 0;
    uint32_t current_tick  = 0;
    int32_t  server_choked = 0;
    bool     round_active  = true;
    int32_t  round_number  = 0;
};

// ---------------------------------------------------------------------------
// planner — исправленный движок оценки
// FIX: RAII + принудительный clear() при смене раунда / изменении тика /
// выходе игрока с сервера (entity cleanup hook).
// ---------------------------------------------------------------------------
class Planner {
public:
    Planner();
    ~Planner();

    Planner(const Planner&)            = delete;
    Planner& operator=(const Planner&) = delete;

    // Точка входа, соответствует оригинальному RVA 0x1E3490
    bool evaluate(const SimInput& in);

    // --- FIX-хуки жизненного цикла (вызываются из подсистем DllMain) ------
    void on_round_start(int32_t round_no);   // сброс BT-истории
    void on_tick_changed(uint32_t new_tick); // усечение scratch-векторов
    void on_entity_gone(int32_t entidx);     // удаление записей игрока
    void shutdown();                         // полная очистка перед unload

    size_t bt_record_count() const { return m_bt_records.capacity(); }
    size_t strategy_count()  const { return m_strategy.size(); }

private:
    void purge_all();

    // В оригинале были сырыми new[]/push_back без очистки на путях выхода.
    // Здесь — контейнеры с явным lifecycle (shrink_to_fit убирает float-мусор).
    std::vector<BtRecord>      m_bt_records;
    std::vector<StrategyEntry> m_strategy;
    std::vector<int32_t>       m_target_cache;

    uint32_t m_last_tick   = 0;
    int32_t  m_last_round  = -1;
    bool     m_initialized = false;
};

// ---------------------------------------------------------------------------
// UI-прослойка (watermark / ImGui-заголовок / Panorama-лог) — ребрендинг
// ---------------------------------------------------------------------------
void        render_watermark(char* out, size_t out_len, int tick, float fps);
const char* menu_title_utf8();           // "NL№1##nlone_main_window"
void        panorama_log(const char* message); // префикс "[NL№1] "

} // namespace nlone
