// SPDX-License-Identifier: AGPL-3.0-only
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#include "../../lib/src/cefwrapper/xfa_print.h"

class CProbeClient : public CefClient, public CefRenderHandler, public CefPrintHandler
{
public:
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
    CefRefPtr<CefPrintHandler> GetPrintHandler() override { return this; }
    void GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect) override
    {
        rect = CefRect(0, 0, 1000, 1400);
    }
    void OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList&,
                 const void*, int, int) override {}
    void OnPrintStart(CefRefPtr<CefBrowser>) override {}
    void OnPrintSettings(CefRefPtr<CefBrowser>, CefRefPtr<CefPrintSettings>, bool) override {}
    void OnPrintReset(CefRefPtr<CefBrowser>) override {}
    bool OnPrintDialog(CefRefPtr<CefBrowser>, bool, CefRefPtr<CefPrintDialogCallback>) override { return false; }
    bool OnPrintJob(CefRefPtr<CefBrowser>, const CefString&, const CefString&,
                    CefRefPtr<CefPrintJobCallback>) override { return false; }
    CefSize GetPdfPaperSize(CefRefPtr<CefBrowser>, int dpi) override
    {
        return CefSize(8.5 * dpi, 11 * dpi);
    }
    IMPLEMENT_REFCOUNTING(CProbeClient);
};

static volatile sig_atomic_t stopped;
static void stop(int) { stopped = 1; }

static GtkPrintUnixDialog* FindDialog()
{
    GList* windows = gtk_window_list_toplevels();
    GtkPrintUnixDialog* result = nullptr;
    for (GList* item = windows; item; item = item->next)
        if (GTK_IS_PRINT_UNIX_DIALOG(item->data))
            result = GTK_PRINT_UNIX_DIALOG(item->data);
    g_list_free(windows);
    return result;
}

int main(int argc, char** argv)
{
    CefMainArgs args(argc, argv);
    const int subprocess = CefExecuteProcess(args, nullptr, nullptr);
    if (subprocess >= 0)
        return subprocess;
    for (const char* name : {"CEF_RESOURCES", "XFA_TEST_PROFILE", "XFA_TEST_URL",
         "XFA_TEST_TRIGGER", "XFA_TEST_OUTPUT", "XFA_TEST_RESULT"})
    {
        if (!getenv(name))
        {
            fprintf(stderr, "Set %s before starting the probe.\n", name);
            return 2;
        }
    }
    const std::string resources = getenv("CEF_RESOURCES");
    const std::string profile = getenv("XFA_TEST_PROFILE");
    const std::string url = getenv("XFA_TEST_URL");
    const std::string trigger = getenv("XFA_TEST_TRIGGER");
    const std::string destination = getenv("XFA_TEST_OUTPUT");
    const std::string resultPath = getenv("XFA_TEST_RESULT");
    const std::string mode = getenv("XFA_TEST_MODE") ? getenv("XFA_TEST_MODE") : "complete";
    const int selectedPage = getenv("XFA_TEST_PAGE") ? atoi(getenv("XFA_TEST_PAGE")) : 0;
    const bool a4 = getenv("XFA_TEST_A4") != nullptr;
    CefSettings settings;
    settings.no_sandbox = true;
    settings.windowless_rendering_enabled = true;
    settings.remote_debugging_port = getenv("XFA_TEST_DEBUG_PORT") ? atoi(getenv("XFA_TEST_DEBUG_PORT")) : 9231;
    CefString(&settings.resources_dir_path) = resources;
    CefString(&settings.locales_dir_path) = resources + "/locales";
    CefString(&settings.user_data_path) = profile;
    CefString(&settings.cache_path) = profile + "/cache";
    CefString(&settings.log_file) = profile + "/cef.log";
    if (!CefInitialize(args, settings, nullptr, nullptr))
        return 2;
    gtk_init(&argc, &argv);
    CefWindowInfo window;
    window.SetAsWindowless(0);
    CefBrowserSettings browserSettings;
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
        window, new CProbeClient(), url, browserSettings, nullptr, nullptr);
    if (!browser)
        return 3;
    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    CefRefPtr<CXfaPrintJob> job;
    bool started = false, configured = false, responded = false;
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds(90);
    while (!stopped && std::chrono::steady_clock::now() < end)
    {
        CefDoMessageLoopWork();
        while (gtk_events_pending())
            gtk_main_iteration_do(FALSE);
        if (!started && access(trigger.c_str(), F_OK) == 0)
        {
            started = true;
            job = new CXfaPrintJob(browser, "XFA regression.pdf", 8.5, 11,
                [&](bool ok) {
                    FILE* result = fopen(resultPath.c_str(), "w");
                    if (result) { fprintf(result, "%d\n", ok); fclose(result); }
                    job = nullptr;
                });
            job->Show();
        }
        GtkPrintUnixDialog* dialog = FindDialog();
        if (dialog && !configured)
        {
            GtkPrintSettings* printSettings = gtk_print_settings_new();
            gtk_print_settings_set_printer(printSettings, "Print to File");
            gtk_print_settings_set(printSettings, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "pdf");
            gchar* uri = g_filename_to_uri(destination.c_str(), nullptr, nullptr);
            gtk_print_settings_set(printSettings, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);
            g_free(uri);
            if (mode == "scale") gtk_print_settings_set_scale(printSettings, 50);
            if (selectedPage)
            {
                GtkPageRange range = {selectedPage - 1, selectedPage - 1};
                gtk_print_settings_set_print_pages(printSettings, GTK_PRINT_PAGES_RANGES);
                gtk_print_settings_set_page_ranges(printSettings, &range, 1);
            }
            gtk_print_unix_dialog_set_settings(dialog, printSettings);
            g_object_unref(printSettings);
            if (a4 || mode == "reverse" || mode == "landscape" || mode == "reverse-landscape" || mode == "margins")
            {
                GtkPageSetup* setup = gtk_page_setup_copy(gtk_print_unix_dialog_get_page_setup(dialog));
                if (a4)
                {
                    GtkPaperSize* paper = gtk_paper_size_new(GTK_PAPER_NAME_A4);
                    gtk_page_setup_set_paper_size(setup, paper);
                    gtk_paper_size_free(paper);
                }
                if (mode == "reverse") gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT);
                if (mode == "landscape") gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_LANDSCAPE);
                if (mode == "reverse-landscape") gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE);
                if (mode == "margins")
                {
                    gtk_page_setup_set_top_margin(setup, 0.5, GTK_UNIT_INCH);
                    gtk_page_setup_set_bottom_margin(setup, 0.5, GTK_UNIT_INCH);
                    gtk_page_setup_set_left_margin(setup, 0.5, GTK_UNIT_INCH);
                    gtk_page_setup_set_right_margin(setup, 0.5, GTK_UNIT_INCH);
                }
                gtk_print_unix_dialog_set_page_setup(dialog, setup);
                g_object_unref(setup);
            }
            configured = true;
        }
        if (dialog && configured && !responded && gtk_print_unix_dialog_get_selected_printer(dialog))
        {
            responded = true;
            if (mode == "close")
            {
                job->Cancel();
                job = nullptr;
                browser->GetHost()->CloseBrowser(true);
            }
            else
            {
                gtk_dialog_response(GTK_DIALOG(dialog), mode == "cancel" ? GTK_RESPONSE_CANCEL : GTK_RESPONSE_OK);
                if (mode == "close-pending")
                {
                    job->Cancel();
                    job = nullptr;
                    browser->GetHost()->CloseBrowser(true);
                }
            }
        }
        usleep(10000);
    }
    if (job) job->Cancel();
    job = nullptr;
    browser->GetHost()->CloseBrowser(true);
    for (int i = 0; i < 200; ++i)
    {
        CefDoMessageLoopWork();
        while (gtk_events_pending()) gtk_main_iteration_do(FALSE);
        usleep(10000);
    }
    browser = nullptr;
    CefShutdown();
    return 0;
}
