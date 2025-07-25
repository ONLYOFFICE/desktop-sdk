HEADERS += \
    $$CEF_SRC_PATH/tests/shared/browser/client_app_browser.h \
    $$CEF_SRC_PATH/tests/shared/browser/extension_util.h \
    $$CEF_SRC_PATH/tests/shared/browser/file_util.h \
    $$CEF_SRC_PATH/tests/shared/browser/geometry_util.h \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop.h \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_external_pump.h \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_std.h \
    $$CEF_SRC_PATH/tests/shared/browser/resource_util.h \
    $$CEF_SRC_PATH/tests/shared/common/binary_value_utils.h \
    $$CEF_SRC_PATH/tests/shared/common/client_app.h \
    $$CEF_SRC_PATH/tests/shared/common/client_app_other.h \
    $$CEF_SRC_PATH/tests/shared/common/client_switches.h \
    $$CEF_SRC_PATH/tests/shared/common/string_util.h \
    $$CEF_SRC_PATH/tests/shared/renderer/client_app_renderer.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/binding_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/bytes_write_handler.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_browser.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_handler.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_handler_std.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_prefs.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_types.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/dialog_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/image_cache.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context_impl.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/media_router_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/preferences_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/resource.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/response_filter_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_manager.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_views.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/scheme_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/server_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/test_runner.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/urlrequest_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_menu_bar.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_overlay_controls.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_style.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_window.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_views.h \
    $$CEF_SRC_PATH/tests/cefclient/common/scheme_test_common.h \
    $$CEF_SRC_PATH/tests/cefclient/renderer/client_renderer.h \
    $$CEF_SRC_PATH/tests/cefclient/renderer/ipc_performance_test.h \
    $$CEF_SRC_PATH/tests/cefclient/renderer/performance_test.h \
    $$CEF_SRC_PATH/tests/cefclient/renderer/performance_test_setup.h

SOURCES += \
    $$CEF_SRC_PATH/tests/shared/browser/client_app_browser.cc \
    $$CEF_SRC_PATH/tests/shared/browser/extension_util.cc \
    $$CEF_SRC_PATH/tests/shared/browser/file_util.cc \
    $$CEF_SRC_PATH/tests/shared/browser/geometry_util.cc \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop.cc \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_external_pump.cc \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_std.cc \
    $$CEF_SRC_PATH/tests/shared/common/binary_value_utils.cc \
    $$CEF_SRC_PATH/tests/shared/common/client_app.cc \
    $$CEF_SRC_PATH/tests/shared/common/client_app_other.cc \
    $$CEF_SRC_PATH/tests/shared/common/client_switches.cc \
    $$CEF_SRC_PATH/tests/shared/common/string_util.cc \
    $$CEF_SRC_PATH/tests/shared/renderer/client_app_renderer.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/binding_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/bytes_write_handler.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_app_delegates_browser.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_browser.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_handler.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_handler_std.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/client_prefs.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/dialog_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/image_cache.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context_impl.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/media_router_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/preferences_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/response_filter_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_create.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_manager.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_views.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/scheme_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/server_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/test_runner.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/urlrequest_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_menu_bar.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_overlay_controls.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_style.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_window.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_views.cc \
    $$CEF_SRC_PATH/tests/cefclient/common/scheme_test_common.cc \
    $$CEF_SRC_PATH/tests/cefclient/renderer/client_renderer.cc \
    $$CEF_SRC_PATH/tests/cefclient/renderer/ipc_performance_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/renderer/performance_test.cc \
    $$CEF_SRC_PATH/tests/cefclient/renderer/performance_test_tests.cc

#HEADERS += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/client_handler_osr.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_dragdrop_events.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_renderer.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_renderer_settings.h

#SOURCES += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/client_handler_osr.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_renderer.cc \
#    $$CEF_SRC_PATH/tests/cefclient/common/client_app_delegates_common.cc \
#    $$CEF_SRC_PATH/tests/cefclient/renderer/client_app_delegates_renderer.cc

core_windows {
HEADERS += \
    $$CEF_SRC_PATH/tests/shared/browser/util_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_message_loop_multithreaded_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_win.h
SOURCES += \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_external_pump_win.cc \
    $$CEF_SRC_PATH/tests/shared/browser/resource_util_win.cc \
    $$CEF_SRC_PATH/tests/shared/browser/util_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context_impl_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_message_loop_multithreaded_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/resource_util_win_idmap.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_win.cc
#HEADERS += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_win.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_helper.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_d3d11_win.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_dragdrop_win.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_ime_handler_win.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_render_handler_win.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_render_handler_win_d3d11.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_render_handler_win_gl.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_window_win.h
#SOURCES += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_win.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_helper.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node_win.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_d3d11_win.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_dragdrop_win.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_ime_handler_win.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_render_handler_win.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_render_handler_win_d3d11.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_render_handler_win_gl.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_window_win.cc \
#    $$CEF_SRC_PATH/tests/cefclient/cefclient_win.cc
}

core_linux {
HEADERS += \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/dialog_handler_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_message_loop_multithreaded_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_x11.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/util_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_gtk.h
SOURCES += \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_external_pump_linux.cc \
    $$CEF_SRC_PATH/tests/shared/browser/resource_util_posix.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/dialog_handler_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context_impl_posix.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_message_loop_multithreaded_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/resource_util_linux.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_x11.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/util_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_gtk.cc
#HEADERS += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_gtk.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/print_handler_gtk.h
#SOURCES += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_gtk.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/print_handler_gtk.cc \
#    $$CEF_SRC_PATH/tests/cefclient/cefclient_gtk.cc
}

core_mac {
HEADERS += \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_mac.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_mac.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_mac.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_mac.h
SOURCES += \
    $$CEF_SRC_PATH/tests/shared/browser/resource_util_posix.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context_impl_posix.cc
OBJECTIVE_SOURCES += \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_external_pump_mac.mm \
    $$CEF_SRC_PATH/tests/shared/browser/resource_util_mac.mm \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_mac.mm \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_mac.mm \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_mac.mm \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_mac.mm
#HEADERS += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_mac.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_helper.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node.h \
#    $$CEF_SRC_PATH/tests/cefclient/browser/text_input_client_osr_mac.h
#SOURCES += \
#    $$CEF_SRC_PATH/tests/shared/process_helper_mac.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_helper.cc \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node.cc
#OBJECTIVE_SOURCES += \
#    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_mac.mm \
#    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node_mac.mm \
#    $$CEF_SRC_PATH/tests/cefclient/browser/text_input_client_osr_mac.mm \
#    $$CEF_SRC_PATH/tests/cefclient/cefclient_mac.mm
}

