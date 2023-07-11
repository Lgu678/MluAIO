#include "../plugin_sdk/plugin_sdk.hpp"
#include "taric.h"
#include <chrono>
#include <array>



using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

namespace taric
{
    TreeTab* main_tab = nullptr;

    script_spell* e = nullptr;
    script_spell* r = nullptr;
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    bool alphaStarted = false;
    bool awaitingEFire = false;
    int timeAlphaStarted = 0;

    namespace ui
    {
        TreeEntry* toggle_stun_keybind = nullptr;
        TreeEntry* toggle_ult_keybind = nullptr;
        TreeEntry* toggle_auto_q = nullptr;
        TreeEntry* q_systems = nullptr; // Checkbox for Q systems
        TreeEntry* toggle_auto_w = nullptr; // Checkbox for automatic W
        TreeEntry* w_health_threshold = nullptr; // Slider for W health threshold
        TreeEntry* toggle_auto_q_lane_clear = nullptr;
        TreeEntry* draw_q_range = nullptr;
        TreeEntry* q_range_color = nullptr;
        TreeEntry* add_colorpick = nullptr;
        TreeEntry* draw_w_range = nullptr;
        TreeEntry* w_range_color = nullptr;
        TreeEntry* draw_e_range = nullptr;
        TreeEntry* e_range_color = nullptr;
        TreeEntry* draw_tethered_ally_circle = nullptr;
        TreeEntry* tethered_ally_circle_color = nullptr;

        TreeEntry* draw_e_range_on_ally = nullptr;
        TreeEntry* e_range_on_ally_color = nullptr;
        TreeEntry* draw_circle_on_enemy_within_e_range = nullptr;
        TreeEntry* enemy_circle_color = nullptr;
        TreeEntry* draw_circle_on_enemy_within_taric_e_range = nullptr;
        TreeEntry* enemy_within_taric_e_range_circle_color = nullptr;


    }

    void on_draw();
    void on_process_spell_cast(game_object_script sender, spell_instance_script spell);
    void on_update();

    void setupMenu()
    {
        main_tab = menu->create_tab("MluAIO", "MluAIO Taric");

        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());

        auto q_icon_texture = myhero->get_spell(spellslot::q)->get_icon_texture();
        auto w_icon_texture = myhero->get_spell(spellslot::w)->get_icon_texture();

        // Get the icon textures for Taric's Q, W, and E spells
        auto e_icon_texture = myhero->get_spell(spellslot::e)->get_icon_texture();
        auto r_icon_texture = myhero->get_spell(spellslot::r)->get_icon_texture();


        // Add main menu settings
        main_tab->add_separator(myhero->get_model() + ".main", "MluAIO Taric");
        main_tab->add_separator(myhero->get_model() + ".invisibleSep", "");
        ui::toggle_stun_keybind = main_tab->add_checkbox(myhero->get_model() + ".stun", "Use E before alpha Animation finishes", true);

        ui::toggle_stun_keybind->set_texture(e_icon_texture);

        ui::toggle_ult_keybind = main_tab->add_checkbox(myhero->get_model() + ".ult.", "Sync Taric R & Yi R", true);

        ui::toggle_ult_keybind->set_texture(r_icon_texture);

        // Add separator for automatic Q settings
        main_tab->add_separator(myhero->get_model() + ".automaticQ", "Automatic Q");

        // Create nested tabs for Q settings
        auto q_auto_tab = main_tab->add_tab("q_auto_tab", "Q Auto Settings");
        q_auto_tab->add_separator(myhero->get_model() + ".separatorTop1", "Automatic Q settings"); // First separator at the top
        q_auto_tab->add_separator(myhero->get_model() + ".separatorTop2", ""); // Second separator at the top
        ui::toggle_auto_q = q_auto_tab->add_checkbox(myhero->get_model() + ".autoQ", "Use Q automatically when orbwalker is in Combo Mode", true);
        ui::toggle_auto_q->set_texture(q_icon_texture);
        q_auto_tab->add_separator(myhero->get_model() + ".separator", "");
        ui::toggle_auto_q_lane_clear = q_auto_tab->add_checkbox(myhero->get_model() + ".autoQ_lane_clear", "Use Q automatically when orbwalker is in lane clear mode", true);
        ui::toggle_auto_q_lane_clear->set_texture(q_icon_texture);
        q_auto_tab->add_separator(myhero->get_model() + ".separatorBottom", ""); // New separator at the bottom
        q_auto_tab->set_assigned_texture(q_icon_texture);

        auto q_combo_tab = main_tab->add_tab("q_combo_tab", "Q Combo Settings");
        ui::q_systems = q_combo_tab->add_checkbox(myhero->get_model() + ".q_systems", "Use Q only when tethered ally is in range", false);
        q_combo_tab->set_assigned_texture(q_icon_texture);

        // Add separator for automatic W settings
        main_tab->add_separator(myhero->get_model() + ".automaticW", "Automatic W");

        // Create nested tabs for W settings
        auto w_auto_tab = main_tab->add_tab("w_auto_tab", "W Auto Settings");
        ui::toggle_auto_w = w_auto_tab->add_checkbox(myhero->get_model() + ".autoW", "Use W automatically when tethered ally's health is low", false);
        ui::toggle_auto_w->set_texture(w_icon_texture);
        w_auto_tab->set_assigned_texture(w_icon_texture);

        auto w_threshold_tab = main_tab->add_tab("w_threshold_tab", "W Threshold Settings");
        ui::w_health_threshold = w_threshold_tab->add_slider(myhero->get_model() + ".wHealthThreshold", "Health threshold (%)", 50, 0, 100);
        ui::w_health_threshold->set_texture(w_icon_texture);
        w_threshold_tab->set_assigned_texture(w_icon_texture);

        // Add separator for Synergy Drawings
        main_tab->add_separator(myhero->get_model() + ".synergyDrawings", "Synergy Drawings");

        // Add "Range Drawings" tab
        auto range_drawings_tab = main_tab->add_tab("range_drawings_tab", "Range Drawings");

        // Q Range Drawing
        range_drawings_tab->add_separator(myhero->get_model() + ".drawQRangeSep", "Q Range Drawing");
        ui::draw_q_range = range_drawings_tab->add_checkbox(myhero->get_model() + ".drawQRange", "Draw Q range", true);
        ui::draw_q_range->set_texture(q_icon_texture);
        ui::q_range_color = range_drawings_tab->add_colorpick(myhero->get_model() + ".qRangeColor", "Q Range Color", { 0.521569f,1.0f,1.0f,0.8f });


        // W Range Drawing
        range_drawings_tab->add_separator(myhero->get_model() + ".drawWRangeSep", "W Range Drawing");
        ui::draw_w_range = range_drawings_tab->add_checkbox(myhero->get_model() + ".drawWRange", "Draw W range", true);
        ui::draw_w_range->set_texture(w_icon_texture);
        ui::w_range_color = range_drawings_tab->add_colorpick(myhero->get_model() + ".wRangeColor", "W Range Color", { 0.521569f,1.0f,1.0f,0.8f });

        // E Range Drawing
        range_drawings_tab->add_separator(myhero->get_model() + ".drawERangeSep", "E Range Drawing");
        ui::draw_e_range = range_drawings_tab->add_checkbox(myhero->get_model() + ".drawERange", "Draw E range", true);
        ui::draw_e_range->set_texture(e_icon_texture);
        ui::e_range_color = range_drawings_tab->add_colorpick(myhero->get_model() + ".eRangeColor", "E Range Color", { 1.0f, 0.1f, 0.1f, 1.0f });

        // Tethered Ally Circle Drawing
        range_drawings_tab->add_separator(myhero->get_model() + ".invisibleSep", "");

        range_drawings_tab->add_separator(myhero->get_model() + ".drawTetheredAllyCircleSep", "Tethered Ally Circle Drawing");

        ui::draw_tethered_ally_circle = range_drawings_tab->add_checkbox(myhero->get_model() + ".drawTetheredAllyCircle", "Draw Circle around Tethered Ally", true);
        ui::tethered_ally_circle_color = range_drawings_tab->add_colorpick(myhero->get_model() + ".tetheredAllyCircleColor", "Tethered Ally Circle Color", { 0.521569f,1.0f,1.0f,0.8f });


        // Draw E Range on Tethered Ally
        ui::draw_e_range_on_ally = range_drawings_tab->add_checkbox(myhero->get_model() + ".drawERangeOnAlly", "Draw E range on Tethered Ally", true);
        ui::draw_e_range_on_ally->set_texture(e_icon_texture);
        ui::e_range_on_ally_color = range_drawings_tab->add_colorpick(myhero->get_model() + ".eRangeOnAllyColor", "E Range on Ally Color", { 1.0f, 0.1f, 0.1f, 1.0f });

        // Draw Circle around Enemy within E range of Ally
        ui::draw_circle_on_enemy_within_e_range = range_drawings_tab->add_checkbox(myhero->get_model() + ".drawCircleOnEnemy", "Draw Circle around Enemy within E range of Ally", true);
        ui::draw_circle_on_enemy_within_e_range->set_texture(e_icon_texture);
        ui::enemy_circle_color = range_drawings_tab->add_colorpick(myhero->get_model() + ".enemyCircleColor", "Enemy Circle Color", { 1.0f, 0.1f, 0.1f, 1.0f });

        // Draw Circle around Enemy within Taric's E range
        ui::draw_circle_on_enemy_within_taric_e_range = range_drawings_tab->add_checkbox(myhero->get_model() + ".drawCircleOnEnemyWithinTaricERange", "Draw Circle around Enemy within Taric's E range", false);
        ui::draw_circle_on_enemy_within_taric_e_range->set_texture(e_icon_texture);
        ui::enemy_within_taric_e_range_circle_color = range_drawings_tab->add_colorpick(myhero->get_model() + ".enemyWithinTaricERangeCircleColor", "Enemy within Taric's E range Circle Color", { 1.0f, 0.1f, 0.1f, 1.0f });

        range_drawings_tab->add_separator(myhero->get_model() + ".invisibleSeparator", "");


        main_tab->add_separator(myhero->get_model() + ".invisibleSeparator", "");
        main_tab->add_separator(myhero->get_model() + ".MluAIO", "MluAIO");

    }




    void load()
    {
        e = plugin_sdk->register_spell(spellslot::e, 1300);
        r = plugin_sdk->register_spell(spellslot::r, 0);
        q = plugin_sdk->register_spell(spellslot::q, 0); // Taric's Q doesn't have a cast range
        w = plugin_sdk->register_spell(spellslot::w, 1300);
        setupMenu();
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_env_draw>::add_callback(on_draw);
        event_handler<events::on_process_spell_cast>::add_callback(on_process_spell_cast);
    }

    void unload()
    {
        menu->delete_tab(main_tab);
        plugin_sdk->remove_spell(e);
        plugin_sdk->remove_spell(r);
        plugin_sdk->remove_spell(q);
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_env_draw>::remove_handler(on_draw);
        event_handler<events::on_process_spell_cast>::remove_handler(on_process_spell_cast);
    }

    void on_process_spell_cast(game_object_script sender, spell_instance_script spell)
    {
        if (ui::toggle_stun_keybind->get_bool() && sender && sender->get_champion() == champion_id::MasterYi && sender->get_team() == myhero->get_team() && spell->get_spellslot() == spellslot::q && !awaitingEFire && !alphaStarted)
        {


            if (!sender->has_buff(buff_hash("taricwallybuff")) || !sender->has_buff(buff_hash("taricwleashactive")) || !e->is_ready() || orbwalker->flee_mode())
                return;
            alphaStarted = true;
            timeAlphaStarted = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        }

        if (ui::toggle_ult_keybind->get_bool() && sender && sender->get_champion() == champion_id::MasterYi && sender->get_team() == myhero->get_team() && spell->get_spellslot() == spellslot::r)
        {
            if (!sender->has_buff(buff_hash("taricwallybuff")) || !sender->has_buff(buff_hash("taricwleashactive")) || !r->is_ready())
                return;

            r->cast();
        }
    }

    game_object_script get_closest_target(float range, vector from)
    {
        auto targets = entitylist->get_enemy_heroes();

        targets.erase(std::remove_if(targets.begin(), targets.end(), [range, from](game_object_script x)
            {
                return !x->is_valid_target(range, from) || x->is_dead();
            }), targets.end());

        std::sort(targets.begin(), targets.end(), [from](game_object_script a, game_object_script b)
            {
                return a->get_distance(from) < b->get_distance(from);
            });

        if (!targets.empty())
            return targets.front();

        return nullptr;
    }

    game_object_script getMasterYi()
    {
        std::vector<game_object_script> allies = entitylist->get_ally_heroes();

        for (int i = 0; i < allies.size(); i++)
        {
            if (allies[i]->get_champion() == champion_id::MasterYi)
            {
                return allies[i];
            }
        }
    }

    void fireTaricE()
    {
        game_object_script target = get_closest_target(575, getMasterYi()->get_position());

        if (target != nullptr) {
            myhero->cast_spell(spellslot::e, target, true, false);
        }

        awaitingEFire = false;
    }

    void on_update()
    {
        if (myhero->is_dead())
        {
            alphaStarted = false;
            awaitingEFire = false;
            return;
        }
        script_spell* w = plugin_sdk->register_spell(spellslot::w, 1300);

        int time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        if (alphaStarted && time - timeAlphaStarted >= 300)
        {
            alphaStarted = false;
            awaitingEFire = true;
        }

        if (awaitingEFire && !getMasterYi()->has_buff(buff_hash("AlphaStrike")))
        {
            fireTaricE();
        }

        // Get the tethered ally
        game_object_script tethered_ally = nullptr;
        auto allies = entitylist->get_ally_heroes();
        for (auto& ally : allies)
        {
            if (ally->has_buff(buff_hash("taricwallybuff")) &&
                ally->has_buff(buff_hash("taricwleashactive")) &&
                ally->get_distance(myhero->get_position()) <= 1300.0f)
            {
                tethered_ally = ally;
                break;
            }
        }

        // Check if tethered ally's health is low, Taric's W is ready and orbwalker is in combo mode
        if (ui::toggle_auto_w->get_bool() && tethered_ally != nullptr &&
            tethered_ally->get_health_percent() <= ui::w_health_threshold->get_int() &&
            w->is_ready())
        {
            if (orbwalker->combo_mode()) // only cast W when orbwalker is in combo mode
            {
                myhero->cast_spell(spellslot::w, tethered_ally);
            }
        }

        if (ui::toggle_auto_q->get_bool() && q->is_ready() && orbwalker->combo_mode())
        {
            if (ui::q_systems->get_bool())
            {
                // get all allies
                auto allies = entitylist->get_ally_heroes();

                // Check if Taric is tethered to any of them and they are within 1300 range
                for (auto& ally : allies)
                {
                    if (ally->get_distance(myhero->get_position()) <= 1300.0f &&
                        ally->has_buff(buff_hash("taricwallybuff")) &&
                        ally->has_buff(buff_hash("taricwleashactive")))
                    {
                        q->cast();
                        break;
                    }
                }
            }
            else
            {
                q->cast();
            }
        }

        // New Q casting condition for lane_clear_mode
        if (ui::toggle_auto_q_lane_clear->get_bool() && q->is_ready() && orbwalker->lane_clear_mode())
        {
            q->cast();
        }
    } // closing bracket for on_update function

    void on_draw()
    {
        if (myhero->is_dead())
            return;

        vector taric_position = myhero->get_position();

        // Q range drawing
        if (ui::draw_q_range->get_bool() && q->is_ready()) {
            unsigned long color = ui::q_range_color->get_color();
            draw_manager->add_circle(taric_position, 325, color, 1.0f);
        }

        // W range drawing
        if (ui::draw_w_range->get_bool() && w->is_ready()) {
            unsigned long color = ui::w_range_color->get_color();
            draw_manager->add_circle(taric_position, 800, color, 1.0f);
        }

        // E range drawing
        if (ui::draw_e_range->get_bool() && e->is_ready()) {
            unsigned long color = ui::e_range_color->get_color();
            draw_manager->add_circle(taric_position, 575, color, 1.0f);
        }

        // Tethered Ally Circle Drawing
        if (ui::draw_tethered_ally_circle->get_bool()) {
            // Get the tethered ally
            game_object_script tethered_ally = nullptr;
            auto allies = entitylist->get_ally_heroes();
            for (auto& ally : allies) {
                if (ally->has_buff(buff_hash("taricwallybuff")) &&
                    ally->has_buff(buff_hash("taricwleashactive")) &&
                    ally->get_distance(myhero->get_position()) <= 1300.0f) {
                    tethered_ally = ally;
                    break;
                }
            }
            // Draw circle around the tethered ally
            if (tethered_ally != nullptr) {
                unsigned long color = ui::tethered_ally_circle_color->get_color();
                draw_manager->add_circle(tethered_ally->get_position(), 200, color, 1.0f);
            }
            // Draw E Range on Tethered Ally
            if (ui::draw_e_range_on_ally->get_bool() && e->is_ready() && tethered_ally != nullptr) {
                unsigned long color = ui::e_range_on_ally_color->get_color();
                draw_manager->add_circle(tethered_ally->get_position(), 575, color, 1.0f);
            }

            // Draw Circle around Enemy within E range of Taric
            if (ui::draw_circle_on_enemy_within_taric_e_range->get_bool() && e->is_ready()) {
                auto enemies = entitylist->get_enemy_heroes();
                for (auto& enemy : enemies) {
                    if (enemy->get_distance(taric_position) <= 575.0f) {
                        unsigned long color = ui::enemy_within_taric_e_range_circle_color->get_color();
                        draw_manager->add_circle(enemy->get_position(), 100, color, 1.0f);

                    }
                    // Draw Circle around Enemy within E range of Ally
                    if (ui::draw_circle_on_enemy_within_e_range->get_bool() && tethered_ally != nullptr) {
                        auto enemies = entitylist->get_enemy_heroes();
                        for (auto& enemy : enemies) {
                            if (enemy->get_distance(tethered_ally->get_position()) <= 575.0f) {
                                unsigned long color = ui::enemy_circle_color->get_color();
                                draw_manager->add_circle(enemy->get_position(), 100, color, 1.0f);
                            }
                        }
                    }
                }
            }
        }
    }



}