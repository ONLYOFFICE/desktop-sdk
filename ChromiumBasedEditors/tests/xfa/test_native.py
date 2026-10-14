#!/usr/bin/env python3
"""Verify CEF downloads and print output using the production URL policy."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import time
import urllib.parse
import urllib.request

from pypdf import PdfReader
from test_viewer import Browser, HERE, dataset_values


def wait_for_browser(endpoint, process):
    deadline = time.monotonic() + 15
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise AssertionError('CEF exited before creating a browser')
        try:
            with urllib.request.urlopen(endpoint + '/json/list', timeout=1) as response:
                if b'webSocketDebuggerUrl' in response.read():
                    return
        except OSError:
            pass
        time.sleep(0.1)
    raise AssertionError('CEF debugging endpoint did not become ready')


def run_case(args, directory, outcome):
    succeeded = outcome == "complete"
    cancel = outcome == "cancel"
    document = HERE / 'fixtures/fixed-unicode-multiline.pdf'
    viewer = args.viewer.resolve()
    url = viewer.as_uri() + '?' + urllib.parse.urlencode({'file': document.as_uri(), 'readonly': 0})
    output = directory / 'filled.pdf'
    printed = directory / 'printed.pdf'
    fallback = directory / 'fallback'
    fallback.mkdir()
    if outcome == 'failed':
        blocker = directory / 'not-a-directory'
        blocker.write_text('block output path')
        output = blocker / 'filled.pdf'
    environment = os.environ.copy()
    environment.update({
        'CEF_RESOURCES': str(args.resources.resolve()),
        'XFA_TEST_PROFILE': str(directory),
        'TMPDIR': str(fallback),
        'XFA_TEST_URL': url,
        'XFA_TEST_SOURCE_URL': document.as_uri(),
        'XFA_TEST_DOWNLOAD_PATH': str(output),
        'XFA_TEST_DEBUG_PORT': str(args.port),
        'XFA_TEST_SECONDS': '90',
    })
    if cancel:
        environment['XFA_TEST_CANCEL_DOWNLOAD'] = '1'
    elif succeeded:
        environment['XFA_TEST_PRINT_PATH'] = str(printed)
    endpoint = 'http://127.0.0.1:' + str(args.port)
    with (directory / 'probe.log').open('w') as log:
        process = subprocess.Popen([
            str(args.probe.resolve()), '--no-sandbox', '--disable-gpu',
            '--password-store=basic', '--allow-file-access-from-files',
            '--disable-extensions', '--disable-plugins', '--disable-site-isolation-trials',
        ], env=environment, stdout=log, stderr=subprocess.STDOUT)
        browser = None
        try:
            wait_for_browser(endpoint, process)
            browser = Browser(endpoint, intercept_downloads=False)
            browser.open(viewer, document)
            value = 'Saved through native CEF download Ω'
            browser.edit_text(value)
            browser.evaluate('XfaViewer.save()')
            browser.wait('testHost.saved.length === 1')
            assert browser.evaluate('testHost.saved[0]') is succeeded
            assert browser.evaluate('testHost.modified') is not succeeded
            assert browser.evaluate('document.getElementById("save").disabled') is False
            if not succeeded:
                assert not output.exists()
                if outcome == 'failed':
                    fallback_file = next(fallback.glob('*.pdf'))
                    assert dataset_values(fallback_file)['unicodeName'] == value
                print('PASS: native ' + outcome + ' download restores save and retains edits')
            else:
                assert dataset_values(output)['unicodeName'] == value
                deadline = time.monotonic() + 20
                while not printed.exists() and time.monotonic() < deadline:
                    time.sleep(0.1)
                pdf = PdfReader(printed)
                assert len(pdf.pages) == 1
                text = pdf.pages[0].extract_text()
                assert 'Saved through native CEF download' in text
                assert 'First line:' in text and 'Second paragraph' in text
                print('PASS: real Blob download reaches native completion and retains XFA')
                print('PASS: native PrintToPDF has one page with edited and multiline text')
            before = browser.evaluate('location.href')
            browser.evaluate('location.href = "https://example.invalid/blocked"')
            time.sleep(0.2)
            assert browser.evaluate('location.href') == before
            other = (HERE / 'fixtures/flowed-controls-repeat.pdf').as_uri()
            assert browser.evaluate('fetch(%r).then(() => false, () => true)' % other)
            print('PASS: production navigation and resource policy blocks other locations')
        except Exception:
            print((directory / 'probe.log').read_text()[-4000:])
            raise
        finally:
            if browser:
                browser.socket.close()
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        log.flush()
    contents = (directory / 'probe.log').read_text()
    assert 'Download started:' in contents
    assert ('Download canceled in dialog:' if cancel else 'Download completed:') in contents


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--resources', type=Path, required=True)
    parser.add_argument('--viewer', type=Path, default=HERE.parent.parent / 'resources/xfa/index.html')
    parser.add_argument('--port', type=int, default=9227)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='onlyoffice-xfa-native-') as temporary:
        for outcome in ('complete', 'cancel', 'failed'):
            directory = Path(temporary) / outcome
            directory.mkdir()
            run_case(args, directory, outcome)


if __name__ == '__main__':
    main()
