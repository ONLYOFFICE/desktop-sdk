# LINUX
HEADERS += \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/dialog_handler_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/print_handler_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_views.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_x11.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_menu_bar.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_style.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_window.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_gtk.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_views.h

SOURCES += \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/dialog_handler_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context_impl_posix.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/print_handler_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/resource_util_linux.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_views.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_x11.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_menu_bar.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_style.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_window.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_gtk.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_views.cc \
    \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_external_pump_linux.cc \
    $$CEF_SRC_PATH/tests/shared/browser/resource_util_posix.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_gtk.cc

CONFIG += linux_3202
linux_3202 {

DEFINES += CEF_3202

HEADERS += \
    $$CEF_SRC_PATH/include/cef_extension.h \
    $$CEF_SRC_PATH/include/cef_extension_handler.h \
    $$CEF_SRC_PATH/include/capi/cef_extension_capi.h \
    $$CEF_SRC_PATH/include/capi/cef_extension_handler_capi.h

HEADERS += \
    $$CEF_SRC_PATH/libcef_dll/cpptoc/extension_handler_cpptoc.h \
    $$CEF_SRC_PATH/libcef_dll/ctocpp/extension_ctocpp.h \
    $$CEF_SRC_PATH/libcef_dll/ctocpp/get_extension_resource_callback_ctocpp.h \
    $$CEF_SRC_PATH/libcef_dll/ctocpp/views/menu_button_pressed_lock_ctocpp.h

SOURCES += \
    $$CEF_SRC_PATH/libcef_dll/cpptoc/extension_handler_cpptoc.cc \
    $$CEF_SRC_PATH/libcef_dll/ctocpp/extension_ctocpp.cc \
    $$CEF_SRC_PATH/libcef_dll/ctocpp/get_extension_resource_callback_ctocpp.cc \
    $$CEF_SRC_PATH/libcef_dll/ctocpp/views/menu_button_pressed_lock_ctocpp.cc

HEADERS += \
    $$CEF_SRC_PATH/tests/cefclient/browser/image_cache.h

SOURCES += \
    $$CEF_SRC_PATH/tests/cefclient/browser/image_cache.cc

HEADERS += \
    $$CEF_SRC_PATH/tests/shared/browser/file_util.h \
    $$CEF_SRC_PATH/tests/shared/browser/extension_util.h

SOURCES += \
    $$CEF_SRC_PATH/tests/shared/browser/file_util.cc \
    $$CEF_SRC_PATH/tests/shared/browser/extension_util.cc

}


