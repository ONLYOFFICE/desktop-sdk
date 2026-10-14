// SPDX-License-Identifier: AGPL-3.0-only
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "include/capi/cef_app_capi.h"
#include "include/capi/cef_client_capi.h"
#include "include/capi/cef_parser_capi.h"
#include "../../lib/src/cefwrapper/xfa_viewer.h"

static cef_client_t client;
static cef_render_handler_t renderer;
static cef_print_handler_t printer;
static cef_pdf_print_callback_t print_result;
static volatile sig_atomic_t stopped;
static cef_request_handler_t requests;
static cef_download_handler_t downloads;
static std::string viewer_url, source_url, download_path, expected_download_url;
static unsigned int download_id;
static bool cancel_download;

static std::string utf8(const cef_string_t *value)
{
    cef_string_utf8_t converted = {0};
    cef_string_utf16_to_utf8(value->str, value->length, &converted);
    std::string result(converted.str, converted.length);
    cef_string_utf8_clear(&converted);
    return result;
}

static std::string canonical_file_url(const std::string& path)
{
    if (path.empty()) return "";
    const std::string input = NSXfaViewer::FileUrl(path);
    cef_string_t address = {0}, canonical = {0};
    cef_urlparts_t parts = {0};
    cef_string_utf8_to_utf16(input.data(), input.size(), &address);
    std::string result;
    if (cef_parse_url(&address, &parts) && cef_create_url(&parts, &canonical))
        result = utf8(&canonical);
    for (cef_string_t *part : {&address, &canonical, &parts.spec, &parts.scheme,
        &parts.username, &parts.password, &parts.host, &parts.port, &parts.origin,
        &parts.path, &parts.query, &parts.fragment})
        cef_string_utf16_clear(part);
    return result;
}

static std::string request_url(cef_request_t *request)
{
    cef_string_userfree_t value = request->get_url(request);
    std::string result = utf8(value);
    cef_string_userfree_utf16_free(value);
    return result;
}

static int CEF_CALLBACK before_browse(cef_request_handler_t *self,
    cef_browser_t *browser, cef_frame_t *frame, cef_request_t *request,
    int user_gesture, int is_redirect)
{
    return !NSXfaViewer::IsViewerUrl(request_url(request), viewer_url);
}

static cef_resource_request_handler_t *CEF_CALLBACK get_resource_handler(
    cef_request_handler_t *self, cef_browser_t *browser, cef_frame_t *frame,
    cef_request_t *request, int is_navigation, int is_download,
    const cef_string_t *initiator, int *disable_default_handling)
{
    *disable_default_handling = !NSXfaViewer::IsAllowedUrl(
        request_url(request), viewer_url, source_url);
    return NULL;
}

static cef_request_handler_t *CEF_CALLBACK get_requests(cef_client_t *self)
{
    return &requests;
}

static cef_download_handler_t *CEF_CALLBACK get_downloads(cef_client_t *self)
{
    return &downloads;
}

static int CEF_CALLBACK can_download(cef_download_handler_t *self,
    cef_browser_t *browser, const cef_string_t *url, const cef_string_t *method)
{
    return utf8(url).find("blob:") == 0;
}

static void finish_download(cef_browser_t *browser, bool complete)
{
    cef_frame_t *frame = browser->get_main_frame(browser);
    cef_string_t code = {0};
    const char *script = complete ?
        "window.XfaViewer.downloadFinished(true)" : "window.XfaViewer.downloadFinished(false)";
    cef_string_utf8_to_utf16(script, strlen(script), &code);
    cef_string_userfree_t url = frame->get_url(frame);
    frame->execute_java_script(frame, &code, url, 0);
    cef_string_userfree_utf16_free(url);
    cef_string_utf16_clear(&code);
    frame->base.release(&frame->base);
}

static void CEF_CALLBACK before_download(cef_download_handler_t *self,
    cef_browser_t *browser, cef_download_item_t *item, const cef_string_t *name,
    cef_before_download_callback_t *callback)
{
    download_id = item->get_id(item);
    expected_download_url.clear();
    fprintf(stderr, "Download started: %u\n", download_id);
    if (cancel_download) {
        fprintf(stderr, "Download canceled in dialog: %u\n", download_id);
        download_id = 0;
        finish_download(browser, false);
        return;
    }
    cef_string_t path = {0};
    cef_string_utf8_to_utf16(download_path.data(), download_path.size(), &path);
    expected_download_url = canonical_file_url(download_path);
    callback->cont(callback, &path, 0);
    cef_string_utf16_clear(&path);
}

static void CEF_CALLBACK download_updated(cef_download_handler_t *self,
    cef_browser_t *browser, cef_download_item_t *item,
    cef_download_item_callback_t *callback)
{
    if (item->get_id(item) != download_id ||
        (!item->is_complete(item) && !item->is_canceled(item) && item->is_in_progress(item)))
        return;
    cef_string_userfree_t path = item->get_full_path(item);
    bool success = item->is_complete(item) && !expected_download_url.empty() &&
        canonical_file_url(utf8(path)) == expected_download_url;
    cef_string_userfree_utf16_free(path);
    fprintf(stderr, "Download completed: %u %d\n", download_id, success);
    download_id = 0;
    expected_download_url.clear();
    finish_download(browser, success);
}

// The harness handlers have process lifetime.
static void CEF_CALLBACK add_ref(cef_base_ref_counted_t *self) {}
static int CEF_CALLBACK release(cef_base_ref_counted_t *self) { return 0; }
static int CEF_CALLBACK has_ref(cef_base_ref_counted_t *self) { return 1; }

static void initialize_base(cef_base_ref_counted_t *base, size_t size)
{
    base->size = size;
    base->add_ref = add_ref;
    base->release = release;
    base->has_one_ref = has_ref;
    base->has_at_least_one_ref = has_ref;
}

static void set_string(cef_string_t *target, const char *value)
{
    cef_string_utf8_to_utf16(value, strlen(value), target);
}

static void CEF_CALLBACK get_view_rect(cef_render_handler_t *self,
                                      cef_browser_t *browser, cef_rect_t *rect)
{
    *rect = {0, 0, 1000, 1400};
}

static void CEF_CALLBACK on_paint(cef_render_handler_t *self,
                                 cef_browser_t *browser,
                                 cef_paint_element_type_t type,
                                 size_t count, const cef_rect_t *rects,
                                 const void *buffer, int width, int height) {}

static cef_render_handler_t *CEF_CALLBACK get_renderer(cef_client_t *self)
{
    return &renderer;
}

static cef_print_handler_t *CEF_CALLBACK get_printer(cef_client_t *self)
{
    return &printer;
}

static cef_size_t CEF_CALLBACK get_paper_size(cef_print_handler_t *self,
                                            cef_browser_t *browser, int dpi)
{
    return {(int)(8.5 * dpi), 11 * dpi};
}

static void CEF_CALLBACK on_print_finished(cef_pdf_print_callback_t *self,
                                         const cef_string_t *path, int ok)
{
    fprintf(stderr, "PDF print completed: %d\n", ok);
}

static void stop(int signal_number) { stopped = 1; }

int main(int argc, char **argv)
{
    cef_main_args_t args = {argc, argv};
    int result = cef_execute_process(&args, NULL, NULL);
    if (result >= 0)
        return result;

    const char *resources = getenv("CEF_RESOURCES");
    const char *profile = getenv("XFA_TEST_PROFILE");
    const char *source = getenv("XFA_TEST_URL");
    char *url = source ? strdup(source) : NULL;
    if (getenv("XFA_TEST_SOURCE_URL")) {
        viewer_url = url ? url : "";
        source_url = getenv("XFA_TEST_SOURCE_URL");
        download_path = getenv("XFA_TEST_DOWNLOAD_PATH") ? getenv("XFA_TEST_DOWNLOAD_PATH") : "";
        cancel_download = getenv("XFA_TEST_CANCEL_DOWNLOAD") != NULL;
    }
    if (!resources || !profile || !url) {
        fprintf(stderr, "Set CEF_RESOURCES, XFA_TEST_PROFILE and XFA_TEST_URL.\n");
        return 2;
    }

    char locales[4096], cache[4096], log_path[4096];
    snprintf(locales, sizeof(locales), "%s/locales", resources);
    snprintf(cache, sizeof(cache), "%s/cache", profile);
    snprintf(log_path, sizeof(log_path), "%s/cef.log", profile);
    cef_settings_t settings = {0};
    settings.size = sizeof(settings);
    settings.no_sandbox = 1;
    settings.windowless_rendering_enabled = 1;
    settings.remote_debugging_port = getenv("XFA_TEST_DEBUG_PORT")
        ? atoi(getenv("XFA_TEST_DEBUG_PORT")) : 9226;
    set_string(&settings.resources_dir_path, resources);
    set_string(&settings.locales_dir_path, locales);
    set_string(&settings.user_data_path, profile);
    set_string(&settings.cache_path, cache);
    set_string(&settings.log_file, log_path);
    if (!cef_initialize(&args, &settings, NULL, NULL))
        return 3;

    initialize_base(&renderer.base, sizeof(renderer));
    renderer.get_view_rect = get_view_rect;
    renderer.on_paint = on_paint;
    initialize_base(&printer.base, sizeof(printer));
    printer.get_pdf_paper_size = get_paper_size;
    initialize_base(&print_result.base, sizeof(print_result));
    print_result.on_pdf_print_finished = on_print_finished;
    initialize_base(&client.base, sizeof(client));
    client.get_render_handler = get_renderer;
    client.get_print_handler = get_printer;
    if (!viewer_url.empty()) {
        initialize_base(&requests.base, sizeof(requests));
        requests.on_before_browse = before_browse;
        requests.get_resource_request_handler = get_resource_handler;
        client.get_request_handler = get_requests;
        initialize_base(&downloads.base, sizeof(downloads));
        downloads.can_download = can_download;
        downloads.on_before_download = before_download;
        downloads.on_download_updated = download_updated;
        client.get_download_handler = get_downloads;
    }

    cef_window_info_t window = {0};
    window.windowless_rendering_enabled = 1;
    cef_browser_settings_t browser_settings = {0};
    browser_settings.size = sizeof(browser_settings);
    browser_settings.windowless_frame_rate = 10;
    cef_string_t address = {0};
    fprintf(stderr, "Opening %s\n", url);
    set_string(&address, url);
    cef_browser_t *browser = cef_browser_host_create_browser_sync(
        &window, &client, &address, &browser_settings, NULL, NULL);
    if (!browser)
        return 4;

    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    int seconds = getenv("XFA_TEST_SECONDS")
        ? atoi(getenv("XFA_TEST_SECONDS")) : 180;
    time_t end = time(NULL) + seconds;
    time_t print_at = time(NULL) + 10;
    const char *print_path = getenv("XFA_TEST_PRINT_PATH");
    cef_browser_host_t *host = browser->get_host(browser);
    while (!stopped && time(NULL) < end) {
        cef_do_message_loop_work();
        usleep(10000);
        if (print_path && time(NULL) >= print_at) {
            cef_string_t path = {0};
            set_string(&path, print_path);
            cef_pdf_print_settings_t pdf_settings = {0};
            pdf_settings.print_background = 1;
            pdf_settings.margin_type = PDF_PRINT_MARGIN_NONE;
            pdf_settings.paper_width = 8.5;
            pdf_settings.paper_height = 11;
            pdf_settings.prefer_css_page_size = 1;
            host->print_to_pdf(host, &path, &pdf_settings, &print_result);
            print_path = NULL;
        }
    }
    host->close_browser(host, 1);
    for (int i = 0; i < 200; ++i) {
        cef_do_message_loop_work();
        usleep(10000);
    }
    host->base.release(&host->base);
    browser->base.release(&browser->base);
    cef_shutdown();
    free(url);
    return 0;
}
