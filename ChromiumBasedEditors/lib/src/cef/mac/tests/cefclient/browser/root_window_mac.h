// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_MAC_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_MAC_H_
#pragma once

#include <string>

#include "include/base/cef_scoped_ptr.h"
#include "tests/cefclient/browser/browser_window.h"
#include "tests/cefclient/browser/root_window.h"

#ifdef __OBJC__
@class NSWindow;
@class NSButton;
@class NSTextField;
#else
class NSWindow;
class NSButton;
class NSTextField;
#endif

namespace client {

// OS X implementation of a top-level native window in the browser process.
// The methods of this class must be called on the main thread unless otherwise
// indicated.
class RootWindowMac : public RootWindow, public BrowserWindow::Delegate {
 public:
  // Constructor may be called on any thread.
  RootWindowMac();
  ~RootWindowMac();

  // RootWindow methods.
  void Init(RootWindow::Delegate* delegate,
            const RootWindowConfig& config,
            const CefBrowserSettings& settings) OVERRIDE;
  void InitAsPopup(RootWindow::Delegate* delegate,
                   bool with_controls,
                   bool with_osr,
                   const CefPopupFeatures& popupFeatures,
                   CefWindowInfo& windowInfo,
                   CefRefPtr<CefClient>& client,
                   CefBrowserSettings& settings) OVERRIDE;
  void Show(ShowMode mode) OVERRIDE;
  void Hide() OVERRIDE;
  void SetBounds(int x, int y, size_t width, size_t height) OVERRIDE;
  void Close(bool force) OVERRIDE;
  void SetDeviceScaleFactor(float device_scale_factor) OVERRIDE;
  float GetDeviceScaleFactor() const OVERRIDE;
  CefRefPtr<CefBrowser> GetBrowser() const OVERRIDE;
  ClientWindowHandle GetWindowHandle() const OVERRIDE;
  bool WithWindowlessRendering() const OVERRIDE;
  bool WithExtension() const OVERRIDE;

  // Called by RootWindowDelegate after the associated NSWindow has been
  // destroyed.
  void WindowDestroyed();

  BrowserWindow* browser_window() const { return browser_window_.get(); }
  RootWindow::Delegate* delegate() const { return delegate_; }

 private:
  void CreateBrowserWindow(const std::string& startup_url);
  void CreateRootWindow(const CefBrowserSettings& settings,
                        bool initially_hidden);

  // BrowserWindow::Delegate methods.
  void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  void OnBrowserWindowDestroyed() OVERRIDE;
  void OnSetAddress(const std::string& url) OVERRIDE;
  void OnSetTitle(const std::string& title) OVERRIDE;
  void OnSetFullscreen(bool fullscreen) OVERRIDE;
  void OnAutoResize(const CefSize& new_size) OVERRIDE;
  void OnSetLoadingState(bool isLoading,
                         bool canGoBack,
                         bool canGoForward) OVERRIDE;
  void OnSetDraggableRegions(
      const std::vector<CefDraggableRegion>& regions) OVERRIDE;

  void NotifyDestroyedIfDone();

  // After initialization all members are only accessed on the main thread.
  // Members set during initialization.
  bool with_controls_;
  bool with_osr_;
  bool with_extension_;
  bool is_popup_;
  CefRect start_rect_;
  scoped_ptr<BrowserWindow> browser_window_;
  bool initialized_;

  // Main window.
  NSWindow* window_;

  // Buttons.
  NSButton* back_button_;
  NSButton* forward_button_;
  NSButton* reload_button_;
  NSButton* stop_button_;

  // URL text field.
  NSTextField* url_textfield_;

  bool window_destroyed_;
  bool browser_destroyed_;

  DISALLOW_COPY_AND_ASSIGN(RootWindowMac);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_MAC_H_
