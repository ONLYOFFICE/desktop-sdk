DEFINES += \
    "_WIN32_WINNT=0x0602" \
    "WINVER=0x0602" \
    "WIN32" \
    "_WINDOWS" \
    "NOMINMAX" \
    "PSAPI_VERSION=1" \
    "__STD_C" \
    "_CRT_SECURE_NO_DEPRECATE" \
    "_SCL_SECURE_NO_DEPRECATE" \
    "NTDDI_VERSION=0x06020000" \
    "_USING_V110_SDK71_" \
    "__STDC_CONSTANT_MACROS" \
    "__STDC_FORMAT_MACROS" \

LIBS += -lwininet \
        -ldnsapi \
        -lversion \
        -lmsimg32 \
        -lws2_32 \
        -lusp10 \
        -lpsapi \
        -ldbghelp \
        -lwinmm \
        -lshlwapi \
        -lkernel32 \
        -lgdi32 \
        -lwinspool \
        -lcomdlg32 \
        -ladvapi32 \
        -lshell32 \
        -lole32 \
        -loleaut32 \
        -luser32 \
        -luuid \
        -lodbc32 \
        -lodbccp32 \
        -ldelayimp \
        -lcredui \
        -lnetapi32 \
        -lcomctl32 \
        -lrpcrt4 \
        -lgdiplus \
        -lUrlmon \
        -lOpenGL32 \
        -lImm32

# WINDOWS
HEADERS += \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_message_loop_multithreaded_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_helper.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_dragdrop_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_ime_handler_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_window_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_views.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_win.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_menu_bar.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_style.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_window.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_views.h \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_win.h \
    \
    $$CEF_SRC_PATH/tests/shared/browser/util_win.h

SOURCES += \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_osr_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_context_impl_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/main_message_loop_multithreaded_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_helper.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_accessibility_node_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_dragdrop_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_ime_handler_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/osr_window_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/resource_util_win_idmap.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_views.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/root_window_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/temp_window_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_menu_bar.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_style.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/views_window.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_views.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/window_test_runner_win.cc \
    \
    $$CEF_SRC_PATH/tests/shared/browser/main_message_loop_external_pump_win.cc \
    $$CEF_SRC_PATH/tests/shared/browser/resource_util_win.cc \
    $$CEF_SRC_PATH/tests/shared/browser/util_win.cc \
    $$CEF_SRC_PATH/tests/cefclient/browser/browser_window_std_win.cc
