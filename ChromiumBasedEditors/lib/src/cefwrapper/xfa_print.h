/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <functional>
#include <string>
#include <utility>
#include <cmath>
#include <unistd.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkunixprint.h>
#include <gdk/gdkx.h>
#undef None
#undef Success
#undef Status
#include "include/cef_browser.h"
#include "include/cef_parser.h"

class CXfaPrintJob : public CefPdfPrintCallback
{
public:
	CXfaPrintJob(CefRefPtr<CefBrowser> browser, const std::string& name,
				double width, double height, std::function<void(bool)> callback)
		: m_browser(browser), m_name(name), m_width(width), m_height(height),
		  m_callback(std::move(callback)) {}

	~CXfaPrintJob()
	{
		if (m_dialog)
			gtk_widget_destroy(m_dialog);
		if (m_job)
			g_object_unref(m_job);
		if (!m_path.empty())
			g_unlink(m_path.c_str());
	}

	void Show()
	{
		m_dialog = gtk_print_unix_dialog_new(m_name.c_str(), nullptr);
		GtkPrintUnixDialog* dialog = GTK_PRINT_UNIX_DIALOG(m_dialog);
		gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
		gtk_print_unix_dialog_set_manual_capabilities(dialog,
			static_cast<GtkPrintCapabilities>(GTK_PRINT_CAPABILITY_GENERATE_PDF | GTK_PRINT_CAPABILITY_SCALE));
		gtk_print_unix_dialog_set_embed_page_setup(dialog, TRUE);
		GtkPageSetup* setup = gtk_page_setup_new();
		GtkPaperSize* paper = nullptr;
		GList* sizes = gtk_paper_size_get_paper_sizes(FALSE);
		for (GList* item = sizes; item && !paper; item = item->next)
		{
			GtkPaperSize* candidate = static_cast<GtkPaperSize*>(item->data);
			const double width = gtk_paper_size_get_width(candidate, GTK_UNIT_INCH);
			const double height = gtk_paper_size_get_height(candidate, GTK_UNIT_INCH);
			if (std::abs(width - m_width) < 0.01 && std::abs(height - m_height) < 0.01)
				paper = gtk_paper_size_copy(candidate);
			else if (std::abs(height - m_width) < 0.01 && std::abs(width - m_height) < 0.01)
			{
				paper = gtk_paper_size_copy(candidate);
				gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_LANDSCAPE);
			}
		}
		g_list_free_full(sizes, reinterpret_cast<GDestroyNotify>(gtk_paper_size_free));
		if (!paper)
			paper = gtk_paper_size_new_custom("xfa", "Form page", m_width, m_height, GTK_UNIT_INCH);
		gtk_page_setup_set_paper_size(setup, paper);
		gtk_paper_size_free(paper);
		gtk_page_setup_set_top_margin(setup, 0, GTK_UNIT_INCH);
		gtk_page_setup_set_bottom_margin(setup, 0, GTK_UNIT_INCH);
		gtk_page_setup_set_left_margin(setup, 0, GTK_UNIT_INCH);
		gtk_page_setup_set_right_margin(setup, 0, GTK_UNIT_INCH);
		gtk_print_unix_dialog_set_page_setup(dialog, setup);
		g_object_unref(setup);
		GtkPrintSettings* settings = gtk_print_settings_new();
		gtk_print_settings_set(settings, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "pdf");
		gtk_print_settings_set(settings, GTK_PRINT_SETTINGS_OUTPUT_BASENAME, m_name.substr(0, m_name.rfind('.')).c_str());
		gtk_print_unix_dialog_set_settings(dialog, settings);
		g_object_unref(settings);
		g_signal_connect(m_dialog, "response", G_CALLBACK(OnResponse), this);
		gtk_widget_realize(m_dialog);
		GdkDisplay* display = gtk_widget_get_display(m_dialog);
		if (GDK_IS_X11_DISPLAY(display))
		{
			Window window = m_browser->GetHost()->GetWindowHandle();
			Window root, parent;
			Window* children = nullptr;
			unsigned int count;
			while (window && XQueryTree(gdk_x11_display_get_xdisplay(display), window, &root, &parent, &children, &count))
			{
				if (children)
					XFree(children);
				if (parent == root)
					break;
				window = parent;
			}
			GdkWindow* owner = window ? gdk_x11_window_foreign_new_for_display(display, window) : nullptr;
			if (owner)
			{
				gdk_window_set_transient_for(gtk_widget_get_window(m_dialog), owner);
				g_object_unref(owner);
			}
		}
		gtk_widget_show(m_dialog);
	}

	void Cancel()
	{
		m_callback = nullptr;
		if (m_dialog)
		{
			gtk_widget_destroy(m_dialog);
			m_dialog = nullptr;
		}
	}

	void OnPdfPrintFinished(const CefString&, bool ok) OVERRIDE
	{
		PreparePrint(nullptr);
		if (!m_callback)
			return;
		GError* error = nullptr;
		if (!ok || !gtk_print_job_set_source_file(m_job, m_path.c_str(), &error))
		{
			g_clear_error(&error);
			Finish(false);
			return;
		}
		AddRef();
		gtk_print_job_send(m_job, OnJobFinished, this, ReleaseJob);
	}

private:
	void Finish(bool ok)
	{
		CefRefPtr<CXfaPrintJob> keepAlive(this);
		auto callback = std::move(m_callback);
		if (callback)
			callback(ok);
	}

	void Respond(int response)
	{
		if (response != GTK_RESPONSE_OK)
		{
			Finish(true);
			return;
		}
		GtkPrintUnixDialog* dialog = GTK_PRINT_UNIX_DIALOG(m_dialog);
		GtkPrinter* printer = gtk_print_unix_dialog_get_selected_printer(dialog);
		if (!printer)
		{
			Finish(false);
			return;
		}
		GtkPrintSettings* settings = gtk_print_unix_dialog_get_settings(dialog);
		const double scale = gtk_print_settings_get_scale(settings);
		GtkPageSetup* setup = gtk_print_unix_dialog_get_page_setup(dialog);
		const double width = gtk_page_setup_get_paper_width(setup, GTK_UNIT_INCH);
		const double height = gtk_page_setup_get_paper_height(setup, GTK_UNIT_INCH);
		const double top = gtk_page_setup_get_top_margin(setup, GTK_UNIT_INCH);
		const double bottom = gtk_page_setup_get_bottom_margin(setup, GTK_UNIT_INCH);
		const double left = gtk_page_setup_get_left_margin(setup, GTK_UNIT_INCH);
		const double right = gtk_page_setup_get_right_margin(setup, GTK_UNIT_INCH);
		if (width <= left + right || height <= top + bottom)
		{
			ShowError("Choose margins smaller than the paper size.");
			g_object_unref(settings);
			return;
		}
		const GtkPageOrientation orientation = gtk_page_setup_get_orientation(setup);
		CefRefPtr<CefDictionaryValue> layout = CefDictionaryValue::Create();
		layout->SetDouble("width", width);
		layout->SetDouble("height", height);
		layout->SetDouble("top", top);
		layout->SetDouble("bottom", bottom);
		layout->SetDouble("left", left);
		layout->SetDouble("right", right);
		layout->SetDouble("scale", scale / 100.0);
		layout->SetBool("reverse", orientation == GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT ||
			orientation == GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE);
		CefPdfPrintSettings pdf;
		pdf.print_background = true;
		pdf.margin_type = PDF_PRINT_MARGIN_NONE;
		pdf.paper_width = width;
		pdf.paper_height = height;
		pdf.prefer_css_page_size = true;
		if (gtk_print_settings_get_print_pages(settings) == GTK_PRINT_PAGES_RANGES)
		{
			int count = 0;
			GtkPageRange* ranges = gtk_print_settings_get_page_ranges(settings, &count);
			std::string pages;
			for (int i = 0; i < count; ++i)
			{
				if (i)
					pages += ",";
				pages += std::to_string(ranges[i].start + 1) + "-" + std::to_string(ranges[i].end + 1);
			}
			g_free(ranges);
			CefString(&pdf.page_ranges) = pages;
		}
		gtk_print_settings_set_print_pages(settings, GTK_PRINT_PAGES_ALL);
		gtk_print_settings_set_scale(settings, 100);
		m_job = gtk_print_job_new(m_name.c_str(), printer, settings, setup);
		g_object_unref(settings);
		gtk_widget_destroy(m_dialog);
		m_dialog = nullptr;
		gchar* path = nullptr;
		GError* error = nullptr;
		const int descriptor = g_file_open_tmp("onlyoffice-xfa-XXXXXX.pdf", &path, &error);
		if (descriptor < 0)
		{
			g_clear_error(&error);
			Finish(false);
			return;
		}
		close(descriptor);
		m_path = path;
		g_free(path);
		PreparePrint(layout);
		m_browser->GetHost()->PrintToPDF(m_path, pdf, this);
	}

	void ShowError(const char* message)
	{
		GtkWidget* error = gtk_message_dialog_new(GTK_WINDOW(m_dialog),
			static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
		g_signal_connect_swapped(error, "response", G_CALLBACK(gtk_widget_destroy), error);
		gtk_widget_show(error);
	}

	void PreparePrint(CefRefPtr<CefDictionaryValue> layout)
	{
		CefRefPtr<CefFrame> frame = m_browser->GetMainFrame();
		if (frame && frame->IsValid())
		{
			CefRefPtr<CefValue> value = CefValue::Create();
			if (layout)
				value->SetDictionary(layout);
			else
				value->SetNull();
			frame->ExecuteJavaScript("window.XfaViewer.preparePrint(" + CefWriteJSON(value, JSON_WRITER_DEFAULT).ToString() + ")",
				frame->GetURL(), 0);
		}
	}

	static void OnResponse(GtkDialog*, gint response, gpointer data)
	{
		static_cast<CXfaPrintJob*>(data)->Respond(response);
	}

	static void OnJobFinished(GtkPrintJob*, gpointer data, const GError* error)
	{
		static_cast<CXfaPrintJob*>(data)->Finish(!error);
	}

	static void ReleaseJob(gpointer data)
	{
		static_cast<CXfaPrintJob*>(data)->Release();
	}

	CefRefPtr<CefBrowser> m_browser;
	std::string m_name;
	double m_width;
	double m_height;
	std::function<void(bool)> m_callback;
	GtkWidget* m_dialog = nullptr;
	GtkPrintJob* m_job = nullptr;
	std::string m_path;
	IMPLEMENT_REFCOUNTING(CXfaPrintJob);
};
