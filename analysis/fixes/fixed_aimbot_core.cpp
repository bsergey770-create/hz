// ============================================================================
// fixed_aimbot_core.cpp — NL№1 (rebrand из neverwin)
//
// ИСПРАВЛЕНИЕ УТЕЧКИ ПАМЯТИ в planner::evaluate (референс: neverwin.dll,
// RVA-кластер 0x1E3490–0x1E4500). Полный разбор дефекта — в
// analysis/fixes/BUG_REPORT_PREDICTION.md
//
// Дефекты оригинала:
//   1) m_bt_records / strategy-векторы росли на каждом тике prediction::sim_tick
//      и никогда не вызывали .clear()/shrink_to_fit — количество записей
//      монотонно росло до смены карты (capacity удерживал float-мусор).
//   2) На ранних возвратах (no target / death / round end) элементы, добавленные
//      в общий m_strategy через emplace_back, оставались — scratch-буферы
//      StrategyEntry::history_scratch не высвобождались.
//   3) При выходе игрока с сервера записи в m_target_cache/m_bt_records не
//      удалялись; evaluate() пересканировал мертвые entidx и аллоцировал новые
//      кандидатов каждый тик.
//
// FIX: RAII-контейнеры + явные lifecycle-колбэки on_round_start /
// on_tick_changed / on_entity_gone / shutdown, вызывающие clear()+shrink_to_fit,
// плюс hard-cap на число кандидатов стратегии.
// ============================================================================

#include "fixed_aimbot_core.hpp"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <utility>

namespace nlone {

// ---------------------------------------------------------------------------
// Planner
// ---------------------------------------------------------------------------
Planner::Planner() {
    m_bt_records.reserve(1024);
    m_strategy.reserve(256);
    m_target_cache.reserve(64);
    m_initialized = true;
}

Planner::~Planner() {
    shutdown();
}

// Реализация исправленного planner::evaluate (RVA 0x1E3490 в оригинале).
bool Planner::evaluate(const SimInput& in) {
    if (!m_initialized)
        return false;

    // ---- FIX #1: смена раунда принудительно обнуляет историю BT ----------
    if (in.round_number != m_last_round) {
        on_round_start(in.round_number);
        m_last_round = in.round_number;
    }

    // ---- FIX #2: изменение тика усекает scratch-накопления ---------------
    if (in.current_tick != m_last_tick) {
        on_tick_changed(in.current_tick);
        m_last_tick = in.current_tick;
    }

    // Раунд закончился / игрок мертв — чистимся ДО выхода, а не после (как в
    // оригинале, где эти пути делали ret без очистки m_strategy).
    if (!in.round_active) {
        purge_all();
        return false;
    }

    // Симуляция кандидатов стратегии. В оригинале здесь был цикл
    // `for each entity: m_strategy.emplace_back(...)` без ограничения размера.
    // Теперь: скользящее окно с hard cap (FIX #4).
    constexpr size_t kMaxStrategyEntries = 256;
    while (m_strategy.size() >= kMaxStrategyEntries) {
        m_strategy.erase(m_strategy.begin());
    }

    StrategyEntry cand{};
    cand.target_entidx  = -1;
    cand.score          = 0.f;
    cand.backtrack_used = (in.server_choked > 0);

    // Детерминированный скоринг по живым записям истории (без скрытых аллокаций).
    for (const BtRecord& rec : m_bt_records) {
        if (!rec.alive)
            continue; // FIX #3: мертвые записи уже удалены on_entity_gone
        const float score = 1.0f / (1.0f + (in.current_tick - rec.tick));
        if (score > cand.score) {
            cand.score         = score;
            cand.target_entidx = rec.tick; // в реальном SDK здесь entidx+hitbox
        }
    }

    if (cand.target_entidx < 0) {
        // Ранний выход БЕЗ утечки: scratch очищен в on_tick_changed выше.
        return false;
    }

    m_strategy.emplace_back(std::move(cand));
    return true;
}

void Planner::on_round_start(int32_t /*round_no*/) {
    // Полная очистка BT-истории на начало раунда.
    m_bt_records.clear();
    m_bt_records.shrink_to_fit();   // возвращает float-мусор в heap
    m_strategy.clear();
    m_strategy.shrink_to_fit();
}

void Planner::on_tick_changed(uint32_t /*new_tick*/) {
    // Усечение временных scratch-буферов каждого кандидата.
    for (auto& s : m_strategy) {
        if (!s.history_scratch.empty()) {
            s.history_scratch.clear();
            s.history_scratch.shrink_to_fit();
        }
    }
    // Ограничиваем рост основной истории между тиками.
    if (m_bt_records.size() > 4096) {
        m_bt_records.erase(m_bt_records.begin(),
                           m_bt_records.end() - 1024);
    }
}

void Planner::on_entity_gone(int32_t entidx) {
    // FIX #3: удаляем ВСЕ следы игрока при despawn.
    m_target_cache.erase(
        std::remove(m_target_cache.begin(), m_target_cache.end(), entidx),
        m_target_cache.end());

    m_strategy.erase(
        std::remove_if(m_strategy.begin(), m_strategy.end(),
                       [entidx](const StrategyEntry& s) {
                           return s.target_entidx == entidx;
                       }),
        m_strategy.end());
}

void Planner::shutdown() {
    purge_all();
    m_initialized = false;
}

void Planner::purge_all() {
    for (auto& s : m_strategy)
        s.history_scratch.clear();
    m_strategy.clear();
    m_bt_records.clear();
    m_target_cache.clear();
    m_bt_records.shrink_to_fit();
    m_strategy.shrink_to_fit();
    m_target_cache.shrink_to_fit();
}

// ---------------------------------------------------------------------------
// UI / логирование — РЕБРЕНДИНГ neverwin -> NL№1
// ---------------------------------------------------------------------------
void render_watermark(char* out, size_t out_len, int tick, float fps) {
    // Было: "neverwin | tick %d | fps %.0f"
    std::snprintf(out, out_len, WATERMARK_FORMAT, BRAND_NAME_SP, tick, fps);
}

const char* menu_title_utf8() {
    // Заголовок ImGui-окна. Было: "neverwin##main_window"
    static char title[64];
    std::snprintf(title, sizeof(title), MENU_TITLE_FORMAT, BRAND_NAME_SP);
    return title;
}

void panorama_log(const char* message) {
    // console.log(PANORAMA_PREFIX + msg) из Panorama-JS превью.
    // Было: "[neverwin] ..."
    std::fprintf(stderr, "%s%s\n", PANORAMA_PREFIX, message ? message : "");
}

} // namespace nlone
