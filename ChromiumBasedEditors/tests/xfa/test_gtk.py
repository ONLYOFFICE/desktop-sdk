#!/usr/bin/env python3
"""Exercise the production GTK print helper with an isolated CEF browser."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import time

from pypdf import PdfReader
from test_native import wait_for_browser
from test_viewer import Browser, HERE

CASES = ('complete', 'page', 'a4', 'landscape', 'reverse', 'reverse-landscape',
         'scale', 'margins', 'cancel', 'failed', 'close', 'close-pending')


def run_case(args, directory, mode):
    metrics = None
    output = directory / 'printed.pdf'
    temporary = directory / 'temporary'
    temporary.mkdir()
    if mode == 'failed':
        blocker = directory / 'not-a-directory'
        blocker.write_text('block output path')
        output = blocker / 'printed.pdf'
    result = directory / 'result'
    trigger = directory / 'trigger'
    environment = os.environ.copy()
    environment.update({
        'CEF_RESOURCES': str(args.resources.resolve()),
        'XFA_TEST_PROFILE': str(directory),
        'TMPDIR': str(temporary),
        'XFA_TEST_URL': 'about:blank',
        'XFA_TEST_DEBUG_PORT': str(args.port),
        'XFA_TEST_TRIGGER': str(trigger),
        'XFA_TEST_OUTPUT': str(output),
        'XFA_TEST_RESULT': str(result),
        'XFA_TEST_MODE': mode,
    })
    if mode == 'page':
        environment['XFA_TEST_PAGE'] = '2'
    if mode == 'a4':
        environment['XFA_TEST_A4'] = '1'
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
            browser = Browser(endpoint)
            browser.open(args.viewer.resolve(), HERE / 'fixtures/flowed-controls-repeat.pdf')
            styles = browser.evaluate('document.querySelectorAll("style").length')
            trigger.touch()
            if mode.startswith('close'):
                time.sleep(3)
                assert not result.exists()
                assert not output.exists()
            else:
                deadline = time.monotonic() + 20
                while not result.exists() and time.monotonic() < deadline:
                    if process.poll() is not None:
                        raise AssertionError('Print helper exited before completion')
                    time.sleep(0.05)
                assert result.read_text().strip() == ('0' if mode == 'failed' else '1')
                if mode in ('cancel', 'failed'):
                    assert not output.exists()
                else:
                    pdf = PdfReader(output)
                    assert len(pdf.pages) == (1 if mode == 'page' else 2)
                    text = ''.join(page.extract_text() for page in pdf.pages)
                    assert 'Row 12' in text
                    assert ('Row 01' in text) is (mode != 'page')
                    for page in pdf.pages:
                        assert not page.images
                        expected = (595.3, 841.9) if mode == 'a4' else (612, 792)
                        if 'landscape' in mode:
                            expected = (792, 612)
                        assert abs(float(page.mediabox.width) - expected[0]) < 1
                        assert abs(float(page.mediabox.height) - expected[1]) < 1
                    text_runs = []
                    pdf.pages[0].extract_text(visitor_text=lambda text, cm, tm, font, size:
                        text_runs.append((text, cm, tm)) if text.strip() else None)
                    assert any(cm[0] < 0 for text, cm, tm in text_runs) is mode.startswith('reverse')
                    if mode in ('complete', 'scale', 'margins'):
                        text, cm, tm = next(run for run in text_runs if 'SYNTHETIC' in run[0])
                        metrics = {'scale': abs(cm[0]),
                                   'x': tm[4] * cm[0] + tm[5] * cm[2] + cm[4],
                                   'y': tm[4] * cm[1] + tm[5] * cm[3] + cm[5]}
                    browser.wait('document.querySelectorAll("style").length === %d' % styles)
            deadline = time.monotonic() + 5
            while list(temporary.glob('onlyoffice-xfa-*.pdf')) and time.monotonic() < deadline:
                time.sleep(0.05)
            assert not list(temporary.glob('onlyoffice-xfa-*.pdf'))
            print('PASS: production GTK print helper ' + mode)
        except Exception:
            print((directory / 'probe.log').read_text()[-5000:])
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
    return metrics


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--resources', type=Path, required=True)
    parser.add_argument('--viewer', type=Path, default=HERE.parent.parent / 'resources/xfa/index.html')
    parser.add_argument('--port', type=int, default=9231)
    parser.add_argument('--case', choices=CASES)
    args = parser.parse_args()
    metrics = {}
    with tempfile.TemporaryDirectory(prefix='onlyoffice-xfa-gtk-') as temporary:
        for mode in [args.case] if args.case else CASES:
            directory = Path(temporary) / mode
            directory.mkdir()
            metrics[mode] = run_case(args, directory, mode)
    if not args.case:
        assert abs(metrics['scale']['scale'] / metrics['complete']['scale'] - 0.5) < 0.01
        assert metrics['scale']['x'] > metrics['complete']['x']
        assert metrics['scale']['y'] < metrics['complete']['y']
        assert metrics['margins']['x'] > metrics['complete']['x']
        assert metrics['margins']['scale'] < metrics['complete']['scale']
        print('PASS: scaling shrinks and centers content; margins move and fit content')


if __name__ == '__main__':
    main()
