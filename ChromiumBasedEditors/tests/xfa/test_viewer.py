#!/usr/bin/env python3
"""Exercise the production XFA viewer in an isolated CEF browser."""
import argparse
import base64
import json
from pathlib import Path
import tempfile
import time
import urllib.parse
import urllib.request
from xml.etree import ElementTree

from pypdf import PdfReader
import websocket

HERE = Path(__file__).resolve().parent
HOST = r"""
window.testHost = {modified: false, saved: [], downloads: [], printed: 0, ready: false};
window.XfaViewerHost = {
  ready() { testHost.ready = true; },
  setModified(value) { testHost.modified = value; },
  saveFinished(value) { testHost.saved.push(value); },
  requestPrint() { testHost.printed++; }
};
HTMLAnchorElement.prototype.click = function () {
  const download = {name: this.download, data: null};
  testHost.downloads.push(download);
  fetch(this.href).then(response => response.arrayBuffer()).then(buffer => {
    const bytes = new Uint8Array(buffer);
    let text = '';
    for (let i = 0; i < bytes.length; i += 32768)
      text += String.fromCharCode(...bytes.subarray(i, i + 32768));
    download.data = btoa(text);
  });
};
"""


class Browser:
    def __init__(self, endpoint, intercept_downloads=True):
        with urllib.request.urlopen(endpoint.rstrip('/') + '/json/list') as response:
            pages = json.load(response)
        page = next(item for item in pages if item['type'] == 'page')
        self.socket = websocket.create_connection(
            page['webSocketDebuggerUrl'], suppress_origin=True, timeout=30)
        self.sequence = 0
        self.call('Page.enable', {})
        host = HOST if intercept_downloads else HOST.split('HTMLAnchorElement.prototype.click')[0]
        self.call('Page.addScriptToEvaluateOnNewDocument', {'source': host})

    def call(self, method, params):
        self.sequence += 1
        request_id = self.sequence
        self.socket.send(json.dumps({'id': request_id, 'method': method, 'params': params}))
        while True:
            result = json.loads(self.socket.recv())
            if result.get('method') == 'Page.javascriptDialogOpening':
                self.sequence += 1
                self.socket.send(json.dumps({'id': self.sequence,
                    'method': 'Page.handleJavaScriptDialog', 'params': {'accept': True}}))
            if result.get('id') == request_id:
                if 'error' in result:
                    raise AssertionError(result['error'])
                return result.get('result', {})

    def evaluate(self, expression):
        result = self.call('Runtime.evaluate', {
            'expression': expression, 'returnByValue': True, 'awaitPromise': True})
        if 'exceptionDetails' in result:
            raise AssertionError(result['exceptionDetails'])
        return result['result'].get('value')

    def wait(self, expression):
        end = time.monotonic() + 15
        while time.monotonic() < end:
            try:
                if self.evaluate(expression):
                    return
            except AssertionError:
                pass
            time.sleep(0.05)
        raise AssertionError('Timed out: ' + expression)

    def open(self, viewer, document, readonly=False):
        query = urllib.parse.urlencode({'file': document.as_uri(), 'readonly': int(readonly)})
        self.call('Page.navigate', {'url': viewer.as_uri() + '?' + query})
        self.wait('testHost.ready === true && document.getElementById("print")?.disabled === false')

    def edit_text(self, value):
        self.evaluate('''(() => {
          const field = document.querySelector('.xfaLayer input:not([type=checkbox]):not([type=radio])');
          field.value = %s;
          field.dispatchEvent(new Event('input', {bubbles: true}));
        })()''' % json.dumps(value))

    def save(self, path):
        count = self.evaluate('testHost.downloads.length')
        self.evaluate('XfaViewer.save()')
        self.wait('testHost.downloads.length > %d && testHost.downloads[%d].data !== null' % (count, count))
        data = self.evaluate('testHost.downloads[%d].data' % count)
        path.write_bytes(base64.b64decode(data))

    def finish(self, success):
        self.evaluate('XfaViewer.downloadFinished(%s)' % json.dumps(success))


def dataset_values(path):
    reader = PdfReader(path)
    packets = reader.trailer['/Root']['/AcroForm']['/XFA']
    data = next(packets[i + 1].get_object().get_data()
                for i in range(0, len(packets), 2) if str(packets[i]) == 'datasets')
    return {node.tag.split('}')[-1]: node.text or ''
            for node in ElementTree.fromstring(data).iter() if not list(node)}


def run(browser, viewer, fixtures, output):
    browser.open(viewer, fixtures / 'fixed-unicode-multiline.pdf')
    assert browser.evaluate('document.querySelectorAll(".sheet").length') == 1
    assert browser.evaluate('document.querySelectorAll(".xfaLayer input, .xfaLayer textarea").length') == 3
    assert browser.evaluate('testHost.modified') is False
    assert browser.evaluate('document.querySelector(".xfaLayer textarea").value') == dataset_values(fixtures / 'fixed-unicode-multiline.pdf')['notes'].replace('\r', '\n')
    untouched = output / 'fixed-untouched.pdf'
    browser.save(untouched)
    browser.finish(True)
    assert untouched.read_bytes() == (fixtures / 'fixed-unicode-multiline.pdf').read_bytes()
    assert browser.evaluate('testHost.modified') is False
    assert browser.evaluate('testHost.saved.at(-1)') is True
    print('PASS: saving an untouched form preserves the original PDF bytes')
    value = 'Jörg Nguyễn — new Ω'
    browser.edit_text(value)
    assert browser.evaluate('testHost.modified') is True
    saved = output / 'fixed-filled.pdf'
    browser.save(saved)
    browser.finish(False)
    assert browser.evaluate('testHost.modified') is True
    assert browser.evaluate('testHost.saved.at(-1)') is False
    assert browser.evaluate('document.getElementById("save").disabled') is False
    browser.save(saved)
    browser.finish(True)
    assert browser.evaluate('testHost.modified') is False
    assert dataset_values(saved)['unicodeName'] == value

    browser.edit_text('First saved revision')
    browser.save(saved)
    browser.edit_text('Later unsaved revision')
    browser.finish(True)
    assert browser.evaluate('testHost.modified') is True
    assert browser.evaluate('testHost.saved.at(-1)') is False
    assert dataset_values(saved)['unicodeName'] == 'First saved revision'
    browser.save(saved)
    browser.finish(True)
    assert browser.evaluate('testHost.modified') is False
    assert dataset_values(saved)['unicodeName'] == 'Later unsaved revision'
    browser.open(viewer, saved)
    assert browser.evaluate('document.querySelector(".xfaLayer input").value') == 'Later unsaved revision'
    print('PASS: Unicode values, native XFA save, reload, canceled/failed save, pending-edit revision')

    browser.open(viewer, fixtures / 'flowed-controls-repeat.pdf')
    assert browser.evaluate('document.querySelectorAll(".sheet").length') == 2
    assert browser.evaluate('document.querySelectorAll(".xfaLayer input, .xfaLayer textarea, .xfaLayer select").length') == 16
    browser.evaluate('''(() => {
      let field = document.querySelector('input[type=checkbox]');
      field.checked = false;
      field.dispatchEvent(new Event('change', {bubbles: true}));
      field = document.querySelector('.xfaLayer select');
      field.value = 'DE';
      field.dispatchEvent(new Event('input', {bubbles: true}));
      field = document.querySelector('input[type=radio]:not(:checked)');
      field.checked = true;
      field.dispatchEvent(new Event('change', {bubbles: true}));
    })()''')
    saved = output / 'controls-filled.pdf'
    browser.save(saved)
    browser.finish(True)
    browser.open(viewer, saved)
    assert browser.evaluate('document.querySelector("input[type=checkbox]").checked') is False
    assert browser.evaluate('document.querySelector(".xfaLayer select").value') == 'DE'
    assert browser.evaluate('document.querySelector("input[type=radio]:checked").getAttribute("xfaOn")') == 'standard'
    browser.evaluate('XfaViewer.print()')
    assert browser.evaluate('testHost.printed') == 1
    assert browser.evaluate('document.getElementById("print").disabled') is True
    browser.evaluate('XfaViewer.printFinished(false)')
    assert browser.evaluate('document.getElementById("print").disabled') is False
    browser.evaluate('XfaViewer.printFinished(false, "Fonts are still loading. Try printing again.")')
    assert browser.evaluate('document.getElementById("status").textContent') == 'Fonts are still loading. Try printing again.'
    assert browser.evaluate('document.getElementById("print").disabled') is False
    browser.evaluate('XfaViewer.print()')
    assert browser.evaluate('testHost.printed') == 2
    assert browser.evaluate('document.getElementById("print").disabled') is True
    browser.evaluate('XfaViewer.printFinished(true)')
    assert browser.evaluate('document.getElementById("print").disabled') is False
    assert browser.evaluate('document.getElementById("status").textContent') == ''
    print('PASS: flowed pages, repeated data, checkbox, dropdown, radio, print request')

    browser.open(viewer, saved, readonly=True)
    assert browser.evaluate('[...document.querySelectorAll(".xfaLayer input, .xfaLayer textarea, .xfaLayer select")].every(field => field.disabled)')
    assert browser.evaluate('document.getElementById("save").hidden') is True
    browser.evaluate('XfaViewer.save()')
    assert browser.evaluate('testHost.downloads.length') == 0
    assert browser.evaluate('testHost.modified') is False
    print('PASS: read-only fields and save guard')

    browser.open(viewer, fixtures / 'access-print-only.pdf')
    assert browser.evaluate('document.querySelector("[xfaName=readOnlyText] input").readOnly')
    assert browser.evaluate('''(() => {
      const fields = [...document.querySelectorAll(
      '.xfaReadOnly select, .xfaReadOnly input[type=checkbox], .xfaDisabled input, .xfaDisabled textarea, .xfaNonInteractive input[type=checkbox]'
      )];
      return fields.length === 5 && fields.every(field => field.disabled);
    })()''')
    assert browser.evaluate('document.querySelector("[xfaName=editableText] input").disabled') is False
    assert browser.evaluate('document.querySelector("[xfaName=editableText] input").readOnly') is False
    assert browser.evaluate('''[...document.querySelectorAll('.xfaLayer :disabled, .xfaLayer [readonly]')]
      .every(field => getComputedStyle(field).backgroundImage === 'none')''')
    assert browser.evaluate('document.querySelectorAll(".xfa-print-only").length') == 2
    assert browser.evaluate('''[...document.querySelectorAll('.xfa-print-only')]
      .every(element => getComputedStyle(element).display === 'none')''')
    browser.call('Emulation.setEmulatedMedia', {'media': 'print'})
    assert browser.evaluate('''[...document.querySelectorAll('.xfa-print-only')]
      .every(element => getComputedStyle(element).display !== 'none')''')
    assert browser.evaluate('getComputedStyle(document.querySelector("[xfaName=printField] .xfaLeft")).display') == 'flex'
    browser.call('Emulation.setEmulatedMedia', {'media': ''})
    field = 'document.querySelector("[xfaName=readOnlyText] input")'
    before = browser.evaluate(field + '.value')
    browser.evaluate(field + '.focus()')
    browser.call('Input.insertText', {'text': 'Blocked edit'})
    assert browser.evaluate(field + '.value') == before
    assert browser.evaluate('getComputedStyle(' + field + ').backgroundColor') == 'rgba(0, 0, 0, 0)'
    point = browser.evaluate('''(() => {
      const field = document.querySelector('[xfaName=readOnlyCheck] input');
      field.scrollIntoView({block: 'center'});
      const rect = field.getBoundingClientRect();
      return {x: rect.x + rect.width / 2, y: rect.y + rect.height / 2};
    })()''')
    for kind in ('mousePressed', 'mouseReleased'):
        browser.call('Input.dispatchMouseEvent', {
            'type': kind, **point, 'button': 'left', 'clickCount': 1})
    assert browser.evaluate('document.querySelector("[xfaName=readOnlyCheck] input").checked')
    assert browser.evaluate('testHost.modified') is False
    editable = 'document.querySelector("[xfaName=editableText] input")'
    assert browser.evaluate('getComputedStyle(' + editable + ').backgroundColor') == 'rgba(153, 193, 255, 0.3)'
    browser.evaluate(editable + '.focus()')
    assert browser.evaluate('getComputedStyle(' + editable + ').backgroundColor') == 'rgba(0, 0, 0, 0)'
    browser.call('Emulation.setEmulatedMedia', {'media': 'print'})
    assert browser.evaluate('getComputedStyle(' + editable + ').outlineStyle') == 'none'
    browser.call('Emulation.setEmulatedMedia', {'media': ''})
    print('PASS: field access, blocked keyboard edit, print-only content and flex layout')

    invalid = output / 'invalid.pdf'
    invalid.write_bytes(b'Invalid PDF test fixture')
    query = urllib.parse.urlencode({'file': invalid.as_uri()})
    browser.call('Page.navigate', {'url': viewer.as_uri() + '?' + query})
    browser.wait('testHost.ready === true')
    assert browser.evaluate('document.getElementById("save").disabled') is True
    assert browser.evaluate('document.getElementById("print").disabled') is True
    assert browser.evaluate('document.getElementById("status").textContent.startsWith("Unable to open")')
    browser.evaluate('XfaViewer.save()')
    assert browser.evaluate('testHost.saved.at(-1)') is False
    assert browser.evaluate('testHost.downloads.length') == 0
    print('PASS: load failure notifies host readiness and keeps actions disabled')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--debug-url', default='http://127.0.0.1:9226')
    parser.add_argument('--viewer', type=Path,
                        default=HERE.parent.parent / 'resources/xfa/index.html')
    parser.add_argument('--fixtures', type=Path, default=HERE / 'fixtures')
    args = parser.parse_args()
    browser = Browser(args.debug_url)
    try:
        with tempfile.TemporaryDirectory(prefix='onlyoffice-xfa-test-') as directory:
            run(browser, args.viewer.resolve(), args.fixtures.resolve(), Path(directory))
    finally:
        browser.socket.close()


if __name__ == '__main__':
    main()
