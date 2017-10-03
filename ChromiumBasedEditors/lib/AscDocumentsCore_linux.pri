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
    $$CEF_SRC_PATH/tests/shared/browser/resource_util_posix.cc

!desktop_sdk_html {
SOURCES += \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_gtk.cc
}

