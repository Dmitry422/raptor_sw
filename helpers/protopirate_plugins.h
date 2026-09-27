#include "../protopirate_app_i.h"

typedef enum ProtoPirateSharedPlugin {
    ProtoPirateSharedPluginsConfig,
    ProtoPirateSharedPluginsSavedInfo,
    ProtoPirateSharedPluginsAbout,
#ifdef ENABLE_EMULATE_FEATURE
    ProtoPirateSharedPluginsEmulate,
#endif
    ProtoPirateSharedPluginsToolScene,
    ProtoPirateSharedPluginsSubDecode,
#ifdef ENABLE_TIMING_TUNER_SCENE
    ProtoPirateSharedPluginsTimingTuner,
#endif
    ProtoPirateSharedPluginsPSABruteforce,
    ProtoPirateSharedPluginsTXRX,
} ProtoPirateSharedPlugin;

bool shared_plugin_load(
    ProtoPirateApp* app,
    ProtoPirateSharedPlugin plugin_type,
    const char* txrx_path);
void shared_plugin_unload(ProtoPirateApp* app, ProtoPirateSharedPlugin plugin_type);
bool shared_plugin_handle_navigation_events(void* context, SceneManagerEvent event);
