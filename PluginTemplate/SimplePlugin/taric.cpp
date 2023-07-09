#include "../plugin_sdk/plugin_sdk.hpp"
#include "taric.h"
#include <chrono>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

namespace taric
{
    TreeTab* main_tab = nullptr;

    script_spell* e = nullptr;
    script_spell* r = nullptr;
    script_spell* q = nullptr;
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

        // Add main menu settings
        main_tab->add_separator(myhero->get_model() + ".main", "MluAIO Taric");
        ui::toggle_stun_keybind = main_tab->add_checkbox(myhero->get_model() + ".stun", "Use E before alpha Animation finishes", true);
        ui::toggle_ult_keybind = main_tab->add_checkbox(myhero->get_model() + ".ult.", "Sync Taric R & Yi R", true);

        // Add separator for automatic Q settings
        main_tab->add_separator(myhero->get_model() + ".automaticQ", "Automatic Q");

        // Create nested tabs for Q settings
        auto q_auto_tab = main_tab->add_tab("q_auto_tab", "Q Auto Settings");
        ui::toggle_auto_q = q_auto_tab->add_checkbox(myhero->get_model() + ".autoQ", "Use Q automatically when orbwalker is in Combo Mode", true);
        q_auto_tab->add_separator(myhero->get_model() + ".separator", "");
        ui::toggle_auto_q_lane_clear = q_auto_tab->add_checkbox(myhero->get_model() + ".autoQ_lane_clear", "Use Q automatically when orbwalker is in lane clear mode", true);
        q_auto_tab->set_assigned_texture(q_icon_texture);

        auto q_combo_tab = main_tab->add_tab("q_combo_tab", "Q Combo Settings");
        ui::q_systems = q_combo_tab->add_checkbox(myhero->get_model() + ".q_systems", "Use Q only when tethered ally is in range", true);
        q_combo_tab->set_assigned_texture(q_icon_texture);

        // Add separator for automatic W settings
        main_tab->add_separator(myhero->get_model() + ".automaticW", "Automatic W");

        // Create nested tabs for W settings
        auto w_auto_tab = main_tab->add_tab("w_auto_tab", "W Auto Settings");
        ui::toggle_auto_w = w_auto_tab->add_checkbox(myhero->get_model() + ".autoW", "Use W automatically when tethered ally's health is low", true);
        w_auto_tab->set_assigned_texture(w_icon_texture);

        auto w_threshold_tab = main_tab->add_tab("w_threshold_tab", "W Threshold Settings");
        ui::w_health_threshold = w_threshold_tab->add_slider(myhero->get_model() + ".wHealthThreshold", "Health threshold (%)", 50, 0, 100);
        w_threshold_tab->set_assigned_texture(w_icon_texture);
    }




    void load()
    {
        e = plugin_sdk->register_spell(spellslot::e, 1300);
        r = plugin_sdk->register_spell(spellslot::r, 0);
        q = plugin_sdk->register_spell(spellslot::q, 0); // Taric's Q doesn't have a cast range
        setupMenu();
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_process_spell_cast>::add_callback(on_process_spell_cast);
    }

    void unload()
    {
        menu->delete_tab(main_tab);
        plugin_sdk->remove_spell(e);
        plugin_sdk->remove_spell(r);
        plugin_sdk->remove_spell(q);
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
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
    }
}
