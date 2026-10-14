# XFA viewer tests

The fixtures contain synthetic fixed and flowed XFA forms. They cover Unicode,
carriage-return paragraphs, repeated records, text fields, checkboxes, dropdowns,
and radio groups. Generated PDFs stay outside version control.

Run from `ChromiumBasedEditors`. Python requires `pypdf` and `websocket-client`.
Prepare the viewer dependency and fixtures first:

```sh
python3 -m pip install pypdf websocket-client
python3 resources/xfa/prepare_vendor.py
python3 tests/xfa/generate_fixtures.py
```

Build the probe against a CEF 109 SDK and matching runtime. `XFA_CEF_SDK` points
to the directory containing `include/`; `XFA_CEF_RUNTIME` contains `libcef.so`,
the resource files and `locales/`. The tests were run with CEF 109.1.18 and
Chromium 109.0.5414.120 on Linux with an X display.

```sh
g++ -std=c++17 -O2 -I"$XFA_CEF_SDK" tests/xfa/cef_probe.cpp \
  -L"$XFA_CEF_RUNTIME" -Wl,-rpath,"$XFA_CEF_RUNTIME" -lcef -o /tmp/xfa-probe
python3 tests/xfa/test_native.py --probe /tmp/xfa-probe --resources "$XFA_CEF_RUNTIME"
```

The native test starts isolated CEF profiles and opens the production `file:`
viewer. It uses the production URL helpers and reproduces the desktop navigation,
resource and download callbacks. It clicks a text field through CEF, types using
keyboard input, saves and reopens the result. It checks real Blob downloads, saved XFA data,
completion, a failed destination, the Save dialog cancellation callback, blocked
navigation and resources, and native `PrintToPDF` output. It does not build the
full desktop application or exercise the Qt dialogs and system printer flow.

For the frontend state tests, start the probe in another terminal:

```sh
export CEF_RESOURCES="$XFA_CEF_RUNTIME"
export XFA_TEST_PROFILE="$(mktemp -d)"
export XFA_TEST_URL=about:blank
/tmp/xfa-probe --disable-gpu --allow-file-access-from-files
```

Then run `python3 tests/xfa/test_viewer.py`. This suite checks values after saving
and reopening, read-only controls, failed or canceled saves, edits made while a
save is pending, and print failure recovery. It intercepts download anchors and
supplies host callbacks so those state transitions can be tested independently.

Both suites use the existing desktop file-access flag and keep web security
enabled. `test_native.py` also keeps extensions and plugins disabled.

The GTK helper test builds against the same CEF SDK, its compiled C++ wrapper
archive (`XFA_CEF_WRAPPER`), GTK3 Unix printing development files, and X11:

```sh
g++ -std=c++17 -O2 -DOVERRIDE=override -I"$XFA_CEF_SDK" tests/xfa/gtk_probe.cpp \
  "$XFA_CEF_WRAPPER" -L"$XFA_CEF_RUNTIME" -Wl,-rpath,"$XFA_CEF_RUNTIME" -lcef \
  $(pkg-config --cflags --libs gtk+-unix-print-3.0) -lX11 -o /tmp/xfa-gtk-probe
python3 tests/xfa/test_gtk.py --probe /tmp/xfa-gtk-probe --resources "$XFA_CEF_RUNTIME"
```

This suite uses the production `CXfaPrintJob` and programmatically selects GTK's
Print to File destination. It checks selectable vector text, page ranges, paper
size, orientation, scale, margins, cancellation, failed output, browser closure,
and temporary file cleanup. It does not submit jobs to a physical printer.
To keep its dialogs off the desktop, run `broadwayd -a 127.0.0.1 -p 18086 :6`
in another terminal and prefix the test command with
`GDK_BACKEND=broadway BROADWAY_DISPLAY=:6`. CEF still needs an X display.

The URL policy tests need only a C++ compiler:

```sh
g++ -std=c++11 -Wall -Wextra -Werror tests/xfa/test_urls.cpp -o /tmp/xfa-urls
/tmp/xfa-urls
g++ -std=c++11 -Wall -Wextra -Werror -D_WIN32 tests/xfa/test_urls.cpp -o /tmp/xfa-urls-win
/tmp/xfa-urls-win
```

For desktop integration, build and package the application with the viewer assets,
then run these checks in an isolated profile:

1. Open a dynamic XFA PDF directly at startup before the profile has a font cache.
   Check that the form renders and the tab finishes loading. Open an ordinary PDF
   and confirm that it still uses the regular PDF editor.
2. Edit a text field and a choice field, save to a new file, and reopen it. Check
   that the values persist and the original file is unchanged. Cancel another
   save and confirm that the edits remain open and a later save succeeds.
3. Close an edited tab and exercise each choice in the native save prompt. Cancel
   must leave the form open. Saving must close the tab only after the file is
   written. Canceling the file dialog must keep the edited tab open.
4. Print the form through the native dialog, check the page count and edited
   values, then cancel another print. Confirm that printing can be started again.
   For PDF output, check the page size and orientation and confirm that text is
   selectable. Change the paper size and scale and verify the resulting layout.
5. Close the application through its normal window controls with both saved and
   unsaved forms. Check that no process remains after accepting the close.
