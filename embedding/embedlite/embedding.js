pref("dom.w3c_touch_events.enabled", 1);
pref("plugin.disable", true);
pref("dom.ipc.plugins.enabled", true);
pref("plugins.force.wmode", "opaque");
pref("browser.xul.error_pages.enabled", true);
pref("gfx.color_management.mode", 0);
pref("nglayout.debug.paint_flashing", false);
pref("nglayout.debug.widget_update_flashing", false);
// Perf trick, speedup motion handlers
pref("layout.reflow.synthMouseMove", false);
// Disable Native themeing because it is usually broken
pref("mozilla.widget.disable-native-theme", true);
// Override some named colors to avoid inverse OS themes
pref("ui.-moz-dialog", "#efebe7");
pref("ui.-moz-dialogtext", "#101010");
pref("ui.-moz-field", "#fff");
pref("ui.-moz-fieldtext", "#1a1a1a");
pref("ui.-moz-buttonhoverface", "#f3f0ed");
pref("ui.-moz-buttonhovertext", "#101010");
pref("ui.-moz-combobox", "#fff");
pref("ui.-moz-comboboxtext", "#101010");
pref("ui.buttonface", "#ece7e2");
pref("ui.buttonhighlight", "#fff");
pref("ui.buttonshadow", "#aea194");
pref("ui.buttontext", "#101010");
pref("ui.captiontext", "#101010");
pref("ui.graytext", "#b1a598");
pref("ui.highlight", "#fad184");
pref("ui.highlighttext", "#1a1a1a");
pref("ui.infobackground", "#f5f5b5");
pref("ui.infotext", "#000");
pref("ui.menu", "#f7f5f3");
pref("ui.menutext", "#101010");
pref("ui.threeddarkshadow", "#000");
pref("ui.threedface", "#ece7e2");
pref("ui.threedhighlight", "#fff");
pref("ui.threedlightshadow", "#ece7e2");
pref("ui.threedshadow", "#aea194");
pref("ui.window", "#efebe7");
pref("ui.windowtext", "#101010");
pref("ui.windowframe", "#efebe7");
// Enable webgl by force
pref("webgl.force-enabled", true);
// Setup dumping enabled for development
pref("browser.dom.window.dump.enabled", true);
pref("layers.acceleration.draw-fps", false);
// SetPref(QString("browser.xul.error_pages.enabled", false);
// Bug 706179 async animation temporary prefs
pref("layers.offmainthreadcomposition.animate-opacity", true);
pref("layers.offmainthreadcomposition.animate-transform", true);
pref("font.size.inflation.disabledInMasterProcess", true);
pref("gfx.azpc.pan_repaint_interval", 5050);
pref("gfx.azpc.fling_repaint_interval", 75);
pref("gfx.use_tiled_thebes", true);
pref("gfx.azpc.y_skate_size_multiplier", "4.5f");
pref("gfx.azpc.y_stationary_size_multiplier", "4.5f");
pref("gfx.axis.max_event_acceleration", "12.0f");
pref("gfx.axis.acceleration_multiplier", "1.125f");
pref("gfx.axis.velocity_multiplier", "0.55f");
pref("gfx.axis.fling_friction", "0.00345f");
pref("gfx.azpc.min_skate_speed", "10.0f");
pref("gfx.azpc.tweak_fling_velocity", false);
pref("gfx.azpc.vertical_scroll_lock_ratio", "1.2f");
pref("ui.dragThresholdX", 25);
pref("ui.dragThresholdY", 25);
pref("embedlite.dispatch_mouse_events", false); // Will dispatch mouse events if page using them
pref("media.gstreamer.enabled", true);
// Disable X backend on GTK
pref("gfx.xrender.enabled", false);
pref("embedlite.azpc.handle.viewport", true);
pref("embedlite.azpc.handle.singletap", true);
pref("embedlite.azpc.handle.longtap", true);
pref("embedlite.azpc.handle.scroll", true);
pref("embedlite.azpc.json.viewport", false);
pref("embedlite.azpc.json.singletap", false);
pref("embedlite.azpc.json.doubletap", false);
pref("embedlite.azpc.json.longtap", false);
pref("embedlite.azpc.json.scroll", false);
pref("extensions.update.enabled", false);
