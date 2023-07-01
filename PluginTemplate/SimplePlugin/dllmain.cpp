#include "../plugin_sdk/plugin_sdk.hpp"
#include "taric.h"

PLUGIN_NAME("MluAIO");
SUPPORTED_CHAMPIONS(champion_id::Taric);

PLUGIN_API bool on_sdk_load(plugin_sdk_core* plugin_sdk_good)
{
    // Global declaretion macro
   //
    DECLARE_GLOBALS(plugin_sdk_good);

    //Switch by myhero champion id
    //
    switch (myhero->get_champion())
    {
    case champion_id::Taric:
        // Load taric script
        //
        taric::load();
        return true;
    default:
        // We don't support this champ, print message and return false (core will not load this plugin and on_sdk_unload will be never called)
        //
        console->print("Champion %s is not supported!", myhero->get_model_cstr());
        return false;
    }

    return false;
}

PLUGIN_API void on_sdk_unload()
{
    switch (myhero->get_champion())
    {
    case champion_id::Taric:
        // Unload taric script
        //
        taric::unload();
        break;
    default:
        break;
    }
}