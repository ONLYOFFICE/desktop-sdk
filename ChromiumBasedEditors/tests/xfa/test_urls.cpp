/* SPDX-License-Identifier: AGPL-3.0-only */
#include "../../lib/src/cefwrapper/xfa_viewer.h"
#include <cassert>

int main()
{
    using namespace NSXfaViewer;
    assert(FileUrl("/tmp/form.pdf") == "file:///tmp/form.pdf");
    assert(FileUrl("/tmp/a #?%.pdf") == "file:///tmp/a%20%23%3F%25.pdf");
    assert(FileUrl("/tmp/Müller.pdf") == "file:///tmp/M%C3%BCller.pdf");
#ifdef _WIN32
    assert(FileUrl("C:\\Users\\Alex\\form.pdf") == "file:///C:/Users/Alex/form.pdf");
    assert(FileUrl("\\\\server\\share\\form.pdf") == "file://server/share/form.pdf");
#else
    assert(FileUrl("/tmp/a\\b.pdf") == "file:///tmp/a%5Cb.pdf");
#endif
    const std::string source = "file:///tmp/form%20one.pdf";
    const std::string viewer = "file:///opt/onlyoffice/editors/xfa/index.html?file=file%3A%2F%2F%2Ftmp%2Fform%20one.pdf";
    assert(IsViewerUrl(viewer, viewer));
    assert(IsViewerUrl(viewer + "#page=2", viewer));
    assert(!IsViewerUrl(source, viewer));
    assert(!IsViewerUrl(viewer + "&file=file:///etc/passwd", viewer));
    assert(IsAllowedUrl(viewer, viewer, source));
    assert(IsAllowedUrl(source, viewer, source));
    assert(IsAllowedUrl("file:///opt/onlyoffice/editors/xfa/pdfjs/legacy/build/pdf.worker.mjs", viewer, source));
    assert(IsAllowedUrl("blob:null/0123", viewer, source));
    assert(IsAllowedUrl("blob:file:///0123", viewer, source));
    assert(IsAllowedUrl("data:image/png;base64,AA==", viewer, source));
    assert(!IsAllowedUrl("https://example.com", viewer, source));
    assert(!IsAllowedUrl("file:///etc/passwd", viewer, source));
    assert(!IsAllowedUrl(source + ".extra", viewer, source));
    assert(!IsAllowedUrl("file:///opt/onlyoffice/editors/xfa-other/index.html", viewer, source));
    assert(!IsAllowedUrl("file:///opt/onlyoffice/editors/xfa/../secret", viewer, source));
    assert(!IsAllowedUrl("file:///opt/onlyoffice/editors/xfa/%2e%2e/secret", viewer, source));
    assert(!IsAllowedUrl("file:///opt/onlyoffice/editors/xfa/fonts%2f..%2fsecret", viewer, source));
    assert(!IsAllowedUrl("file:///opt/onlyoffice/editors/xfa/fonts%5c..%5csecret", viewer, source));
    assert(!IsAllowedUrl("file:///opt/onlyoffice/editors/secret", viewer, source));
    return 0;
}
