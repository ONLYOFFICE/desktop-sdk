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
resource and download callbacks. It checks real Blob downloads, saved XFA data,
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

The URL policy tests need only a C++ compiler:

```sh
g++ -std=c++11 -Wall -Wextra -Werror tests/xfa/test_urls.cpp -o /tmp/xfa-urls
/tmp/xfa-urls
g++ -std=c++11 -Wall -Wextra -Werror -D_WIN32 tests/xfa/test_urls.cpp -o /tmp/xfa-urls-win
/tmp/xfa-urls-win
```
