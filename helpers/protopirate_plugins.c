#include "protopirate_plugins.h"
#include <loader/firmware_api/firmware_api.h>

// -----------------------------------------------------------------------------
// Plugin load / unload
// -----------------------------------------------------------------------------
const FlipperAppPluginDescriptor* load_plugin_fal(
    FlipperApplication** fal_app,
    const char* plugin_path,
    const char* application_id,
    uint32_t api_version) {
    const FlipperAppPluginDescriptor* app_descriptor;

    bool return_value = false;
    do {
        FlipperApplicationPreloadStatus preload_res =
            flipper_application_preload(*fal_app, plugin_path);
        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to preload plugin: %s", plugin_path);
            break;
        }

        if(!flipper_application_is_plugin(*fal_app)) {
            FURI_LOG_E(TAG, "Plugin file is not a library");
            break;
        }

        FlipperApplicationLoadStatus load_status = flipper_application_map_to_memory(*fal_app);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to load plugin file");
            break;
        }

        FURI_LOG_D(TAG, "is mapped to memory");
        app_descriptor = flipper_application_plugin_get_descriptor(*fal_app);
        if(strcmp(app_descriptor->appid, application_id) != 0) {
            FURI_LOG_E(TAG, "Application id mismatch %s", application_id);
            break;
        }

        if(app_descriptor->ep_api_version != api_version) {
            FURI_LOG_E(TAG, "API version mismatch %lu", api_version);
            break;
        }

        FURI_LOG_I(
            TAG,
            "Loaded plugin for appid '%s', API %lu",
            app_descriptor->appid,
            app_descriptor->ep_api_version);
        return_value = true;
    } while(false);

    //Free the applciation if there was an error, otherwise return the app descriptor.
    if(return_value) {
        return app_descriptor;
    } else {
        if(*fal_app) {
            flipper_application_free(*fal_app);
            *fal_app = NULL;
        }
        return NULL;
    }
}

void shared_plugin_unload(ProtoPirateApp* app, ProtoPirateSharedPlugin plugin_type) {
    //Clear the invalid reference to the plugin.
    FlipperApplication** fal_needs_free = NULL;
    switch(plugin_type) {
    case ProtoPirateSharedPluginsConfig: {
        fal_needs_free = &app->plugin_flipper_application;
        app->config_plugin = NULL;
        break;
    }
    case ProtoPirateSharedPluginsSavedInfo: {
        fal_needs_free = &app->plugin_flipper_application;
        app->saved_info_plugin = NULL;
        break;
    }
    case ProtoPirateSharedPluginsAbout: {
        fal_needs_free = &app->plugin_flipper_application;
        app->about_plugin = NULL;
        break;
    }
#ifdef ENABLE_EMULATE_FEATURE
    case ProtoPirateSharedPluginsEmulate: {
        fal_needs_free = &app->plugin_flipper_application;
        app->emulate_plugin = NULL;
        break;
    }
#endif
    case ProtoPirateSharedPluginsToolScene:
    case ProtoPirateSharedPluginsSubDecode:
#ifdef ENABLE_TIMING_TUNER_SCENE
    case ProtoPirateSharedPluginsTimingTuner:
#endif
    {
        fal_needs_free = &app->tool_scene_plugin_flipper_application;
        app->tool_scene_plugin = NULL;
        break;
    }
    case ProtoPirateSharedPluginsPSABruteforce: {
        fal_needs_free = &app->psa_bf_plugin_flipper_application;
        app->psa_bf_plugin = NULL;
        break;
    }
    case ProtoPirateSharedPluginsTXRX: {
        fal_needs_free = &app->txrx->protocol_plugin_flipper_application;
        app->txrx->protocol_plugin = NULL;
        break;
    }
    default:
        return;
    }

    //Free the flipper application.
    if(*fal_needs_free) {
        flipper_application_free(*fal_needs_free);
        *fal_needs_free = NULL;
    }
}

bool shared_plugin_load(
    ProtoPirateApp* app,
    ProtoPirateSharedPlugin plugin_type,
    const char* txrx_path) {
    //Get the APPID and API VERSION for the Plugin we are loading.
    const char* application_id = NULL;
    const char* plugin_path = NULL;
    uint32_t api_version = 0;
    FlipperApplication** fal_needs_alloc = NULL;

    switch(plugin_type) {
    case ProtoPirateSharedPluginsConfig: {
        if(app->config_plugin) return true;
        application_id = PROTOPIRATE_CONFIG_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_CONFIG_PLUGIN_API_VERSION;
        plugin_path = CONFIG_PLUGIN_PATH;
        fal_needs_alloc = &app->plugin_flipper_application;
        break;
    }
    case ProtoPirateSharedPluginsSavedInfo: {
        if(app->saved_info_plugin) return true;
        application_id = PROTOPIRATE_SAVED_INFO_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_SAVED_INFO_PLUGIN_API_VERSION;
        plugin_path = SAVED_INFO_PLUGIN_PATH;
        fal_needs_alloc = &app->plugin_flipper_application;
        break;
    }
    case ProtoPirateSharedPluginsAbout: {
        if(app->about_plugin) return true;
        application_id = PROTOPIRATE_ABOUT_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_ABOUT_PLUGIN_API_VERSION;
        plugin_path = ABOUT_PLUGIN_PATH;
        fal_needs_alloc = &app->plugin_flipper_application;
        break;
    }
#ifdef ENABLE_EMULATE_FEATURE
    case ProtoPirateSharedPluginsEmulate: {
        if(app->emulate_plugin) return true;
        application_id = PROTOPIRATE_EMULATE_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_EMULATE_PLUGIN_API_VERSION;
        plugin_path = EMULATE_PLUGIN_PATH;
        fal_needs_alloc = &app->plugin_flipper_application;
        break;
    }
#endif
    case ProtoPirateSharedPluginsSubDecode: {
        if(app->tool_scene_plugin) return true;
        application_id = PROTOPIRATE_TOOL_SCENE_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_TOOL_SCENE_PLUGIN_API_VERSION;
        plugin_path = SUB_DECODE_PLUGIN_PATH;
        fal_needs_alloc = &app->tool_scene_plugin_flipper_application;
        break;
    }
#ifdef ENABLE_TIMING_TUNER_SCENE
    case ProtoPirateSharedPluginsTimingTuner: {
        if(app->tool_scene_plugin) return true;
        application_id = PROTOPIRATE_TOOL_SCENE_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_TOOL_SCENE_PLUGIN_API_VERSION;
        plugin_path = TIMING_TUNER_PLUGIN_PATH;
        fal_needs_alloc = &app->tool_scene_plugin_flipper_application;
        break;
    }
#endif
    case ProtoPirateSharedPluginsPSABruteforce: {
        if(app->psa_bf_plugin) return true;
        application_id = PROTOPIRATE_PSA_BF_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_PSA_BF_PLUGIN_API_VERSION;
        plugin_path = PSA_BF_PLUGIN_PATH;
        fal_needs_alloc = &app->psa_bf_plugin_flipper_application;
        break;
    }
    case ProtoPirateSharedPluginsTXRX: {
        if(app->txrx->protocol_plugin) return true;
        application_id = PROTOPIRATE_PROTOCOL_PLUGIN_APP_ID;
        api_version = PROTOPIRATE_PROTOCOL_PLUGIN_API_VERSION;
        plugin_path = txrx_path;
        fal_needs_alloc = &app->txrx->protocol_plugin_flipper_application;
        break;
    }
    default:
        return false;
    }

    FURI_LOG_D(TAG, "Loading plugin: %s", plugin_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperApplication* fal_app = flipper_application_alloc(storage, firmware_api_interface);
    bool return_value = false;
    do {
        //Load the FAP
        const FlipperAppPluginDescriptor* app_descriptor =
            load_plugin_fal(&fal_app, plugin_path, application_id, api_version);
        if(app_descriptor) {
            //Get the Plugin and assign it.
            if(plugin_type == ProtoPirateSharedPluginsConfig) {
                const ProtoPirateConfigPlugin* plugin_config = app_descriptor->entry_point;
                if(!plugin_config || !plugin_config->on_enter) {
                    FURI_LOG_E(TAG, "Config plugin entry point is invalid");
                } else {
                    app->config_plugin = plugin_config;
                    return_value = true;
                }
            } else if(plugin_type == ProtoPirateSharedPluginsSavedInfo) {
                const ProtoPirateSavedInfoPlugin* plugin_saved_info = app_descriptor->entry_point;
                if(!plugin_saved_info || !plugin_saved_info->on_enter) {
                    FURI_LOG_E(TAG, "Saved Info plugin entry point is invalid");
                } else {
                    app->saved_info_plugin = plugin_saved_info;
                    return_value = true;
                }
            } else if(plugin_type == ProtoPirateSharedPluginsAbout) {
                const ProtoPirateAboutPlugin* plugin_about = app_descriptor->entry_point;
                if(!plugin_about || !plugin_about->on_enter) {
                    FURI_LOG_E(TAG, "About plugin entry point is invalid");
                } else {
                    app->about_plugin = plugin_about;
                    return_value = true;
                }
            }
#ifdef ENABLE_EMULATE_FEATURE
            else if(plugin_type == ProtoPirateSharedPluginsEmulate) {
                const ProtoPirateEmulatePlugin* plugin_emulate = app_descriptor->entry_point;
                if(!plugin_emulate || !plugin_emulate->on_enter) {
                    FURI_LOG_E(TAG, "Emulate plugin entry point is invalid");
                } else {
                    app->emulate_plugin = plugin_emulate;
                    return_value = true;
                }
            }
#endif
            else if(
                plugin_type == ProtoPirateSharedPluginsSubDecode
#ifdef ENABLE_TIMING_TUNER_SCENE
                || plugin_type == ProtoPirateSharedPluginsTimingTuner
#endif
            ) {
                const ProtoPirateToolScenePlugin* plugin_tool_scene = app_descriptor->entry_point;
                if(!plugin_tool_scene || !plugin_tool_scene->on_enter) {
                    FURI_LOG_E(TAG, "Tool Scene plugin entry point is invalid");
                } else {
                    app->tool_scene_plugin = plugin_tool_scene;
                    return_value = true;
                }
            } else if(plugin_type == ProtoPirateSharedPluginsPSABruteforce) {
                const ProtoPiratePsaBfPlugin* plugin_psa_bf = app_descriptor->entry_point;
                if(!plugin_psa_bf || !plugin_psa_bf->needs_bruteforce) {
                    FURI_LOG_E(TAG, "PSA plugin entry needs_bruteforce is invalid");
                } else {
                    app->psa_bf_plugin = plugin_psa_bf;
                    return_value = true;
                }
            } else if(plugin_type == ProtoPirateSharedPluginsTXRX) {
                const ProtoPirateProtocolPlugin* plugin_txrx = app_descriptor->entry_point;
                if(!plugin_txrx || !plugin_txrx->registry) {
                    FURI_LOG_E(TAG, "Protocol plugin registry entry is invalid");
                } else {
                    app->txrx->protocol_plugin = plugin_txrx;
                    return_value = true;
                }
            }
        }
    } while(false);

    //Free the plugin if there was an error, otherwise we are done!
    if(fal_needs_alloc) *fal_needs_alloc = fal_app;
    furi_record_close(RECORD_STORAGE);
    return return_value;
}

bool shared_plugin_handle_navigation_events(void* context, SceneManagerEvent event) {
    ProtoPirateApp* app = (ProtoPirateApp*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == ProtoPirateCustomEventPluginNavigateEmulate) {
#ifdef ENABLE_EMULATE_FEATURE
            scene_manager_next_scene(app->scene_manager, ProtoPirateSceneEmulate);
#endif
            return true;
        }
        if(event.event == ProtoPirateCustomEventPluginNavigateConfig) {
            scene_manager_next_scene(app->scene_manager, ProtoPirateSceneReceiverConfig);
            return true;
        } else if(event.event == ProtoPirateCustomEventPluginNavigateBack) {
            scene_manager_previous_scene(app->scene_manager);
            return true;
        } else if(event.event == ProtoPirateCustomEventPluginNavigateStopApp) {
            scene_manager_stop(app->scene_manager);
            view_dispatcher_stop(app->view_dispatcher);
            return true;
        }
    }
    return false;
}
