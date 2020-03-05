HEADERS += \
    $$CEF_SRC_PATH/cefclient/browser/binding_test.h \
    $$CEF_SRC_PATH/cefclient/browser/browser_window.h \
    $$CEF_SRC_PATH/cefclient/browser/browser_window_std_win.h \
    $$CEF_SRC_PATH/cefclient/browser/bytes_write_handler.h \
    $$CEF_SRC_PATH/cefclient/browser/client_app_browser.h \
    $$CEF_SRC_PATH/cefclient/browser/client_handler.h \
    $$CEF_SRC_PATH/cefclient/browser/client_handler_std.h \
    $$CEF_SRC_PATH/cefclient/browser/client_types.h \
    $$CEF_SRC_PATH/cefclient/browser/dialog_test.h \
    $$CEF_SRC_PATH/cefclient/browser/geometry_util.h \
    $$CEF_SRC_PATH/cefclient/browser/main_context.h \
    $$CEF_SRC_PATH/cefclient/browser/main_context_impl.h \
    $$CEF_SRC_PATH/cefclient/browser/main_message_loop.h \
    $$CEF_SRC_PATH/cefclient/browser/main_message_loop_multithreaded_win.h \
    $$CEF_SRC_PATH/cefclient/browser/main_message_loop_std.h \
    $$CEF_SRC_PATH/cefclient/browser/preferences_test.h \
    $$CEF_SRC_PATH/cefclient/browser/resource.h \
    $$CEF_SRC_PATH/cefclient/browser/resource_util.h \
    $$CEF_SRC_PATH/cefclient/browser/response_filter_test.h \
    $$CEF_SRC_PATH/cefclient/browser/root_window.h \
    $$CEF_SRC_PATH/cefclient/browser/root_window_manager.h \
    $$CEF_SRC_PATH/cefclient/browser/root_window_win.h \
    $$CEF_SRC_PATH/cefclient/browser/scheme_test.h \
    $$CEF_SRC_PATH/cefclient/browser/temp_window.h \
    $$CEF_SRC_PATH/cefclient/browser/temp_window_win.h \
    $$CEF_SRC_PATH/cefclient/browser/test_runner.h \
    $$CEF_SRC_PATH/cefclient/browser/urlrequest_test.h \
    $$CEF_SRC_PATH/cefclient/browser/util_win.h \
    $$CEF_SRC_PATH/cefclient/browser/window_test.h \
    $$CEF_SRC_PATH/cefclient/common/client_app.h \
    $$CEF_SRC_PATH/cefclient/common/client_app_other.h \
    $$CEF_SRC_PATH/cefclient/common/client_switches.h \
    $$CEF_SRC_PATH/cefclient/common/scheme_test_common.h \
    $$CEF_SRC_PATH/cefclient/renderer/client_app_renderer.h \
    $$CEF_SRC_PATH/cefclient/renderer/client_renderer.h \
    $$CEF_SRC_PATH/cefclient/renderer/performance_test.h \
    $$CEF_SRC_PATH/cefclient/renderer/performance_test_setup.h

SOURCES += \
    $$CEF_SRC_PATH/cefclient/browser/binding_test.cc \
    $$CEF_SRC_PATH/cefclient/browser/browser_window.cc \
    $$CEF_SRC_PATH/cefclient/browser/browser_window_std_win.cc \
    $$CEF_SRC_PATH/cefclient/browser/bytes_write_handler.cc \
    $$CEF_SRC_PATH/cefclient/browser/client_app_browser.cc \
    $$CEF_SRC_PATH/cefclient/browser/client_app_delegates_browser.cc \
    $$CEF_SRC_PATH/cefclient/browser/client_handler.cc \
    $$CEF_SRC_PATH/cefclient/browser/client_handler_std.cc \
    $$CEF_SRC_PATH/cefclient/browser/dialog_test.cc \
    $$CEF_SRC_PATH/cefclient/browser/geometry_util.cc \
    $$CEF_SRC_PATH/cefclient/browser/main_context.cc \
    $$CEF_SRC_PATH/cefclient/browser/main_context_impl.cc \
    $$CEF_SRC_PATH/cefclient/browser/main_context_impl_win.cc \
    $$CEF_SRC_PATH/cefclient/browser/main_message_loop.cc \
    $$CEF_SRC_PATH/cefclient/browser/main_message_loop_multithreaded_win.cc \
    $$CEF_SRC_PATH/cefclient/browser/main_message_loop_std.cc \
    $$CEF_SRC_PATH/cefclient/browser/preferences_test.cc \
    $$CEF_SRC_PATH/cefclient/browser/resource_util_win.cc \
    $$CEF_SRC_PATH/cefclient/browser/response_filter_test.cc \
    $$CEF_SRC_PATH/cefclient/browser/root_window.cc \
    $$CEF_SRC_PATH/cefclient/browser/root_window_manager.cc \
    $$CEF_SRC_PATH/cefclient/browser/root_window_win.cc \
    $$CEF_SRC_PATH/cefclient/browser/scheme_test.cc \
    $$CEF_SRC_PATH/cefclient/browser/temp_window_win.cc \
    $$CEF_SRC_PATH/cefclient/browser/test_runner.cc \
    $$CEF_SRC_PATH/cefclient/browser/urlrequest_test.cc \
    $$CEF_SRC_PATH/cefclient/browser/util_win.cc \
    $$CEF_SRC_PATH/cefclient/browser/window_test.cc \
    $$CEF_SRC_PATH/cefclient/browser/window_test_win.cc \
    $$CEF_SRC_PATH/cefclient/common/client_app.cc \
    $$CEF_SRC_PATH/cefclient/common/client_app_other.cc \
    $$CEF_SRC_PATH/cefclient/common/client_switches.cc \
    $$CEF_SRC_PATH/cefclient/common/scheme_test_common.cc \
    $$CEF_SRC_PATH/cefclient/renderer/client_app_renderer.cc \
    $$CEF_SRC_PATH/cefclient/renderer/client_renderer.cc \
    $$CEF_SRC_PATH/cefclient/renderer/performance_test.cc \
    $$CEF_SRC_PATH/cefclient/renderer/performance_test_tests.cc

#HEADERS += \
#    $$CEF_SRC_PATH/cefclient/browser/browser_window_osr_win.h \
#    $$CEF_SRC_PATH/cefclient/browser/client_handler_osr.h \
#    $$CEF_SRC_PATH/cefclient/browser/osr_dragdrop_events.h \
#    $$CEF_SRC_PATH/cefclient/browser/osr_dragdrop_win.h \
#    $$CEF_SRC_PATH/cefclient/browser/osr_renderer.h \
#    $$CEF_SRC_PATH/cefclient/browser/osr_window_win.h

#SOURCES += \
#    $$CEF_SRC_PATH/cefclient/browser/browser_window_osr_win.cc \
#    $$CEF_SRC_PATH/cefclient/browser/client_handler_osr.cc \
#    $$CEF_SRC_PATH/cefclient/browser/osr_dragdrop_win.cc \
#    $$CEF_SRC_PATH/cefclient/browser/osr_renderer.cc \
#    $$CEF_SRC_PATH/cefclient/browser/osr_window_win.cc \
#    $$CEF_SRC_PATH/cefclient/cefclient_win.cc \
#    $$CEF_SRC_PATH/cefclient/common/client_app_delegates_common.cc \
#    $$CEF_SRC_PATH/cefclient/renderer/client_app_delegates_renderer.cc

