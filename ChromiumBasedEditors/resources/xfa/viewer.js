/* SPDX-License-Identifier: AGPL-3.0-only */
import { getDocument, GlobalWorkerOptions, XfaLayer } from './pdfjs/legacy/build/pdf.mjs';

GlobalWorkerOptions.workerSrc = new URL('pdfjs/legacy/build/pdf.worker.mjs', import.meta.url).href;
const parameters = new URLSearchParams(location.search);
const source = new URL(parameters.get('file'));
const readOnly = parameters.get('readonly') === '1';
const pages = document.getElementById('pages');
const status = document.getElementById('status');
const saveButton = document.getElementById('save');
const printButton = document.getElementById('print');
const filename = decodeURIComponent(source.pathname.split('/').pop());
document.title = filename;
document.getElementById('filename').textContent = filename;
saveButton.hidden = readOnly;

let pdf;
let ready = false;
let revision = 0;
let savedRevision = 0;
let pendingSave = null;

function reportModified() {
  const modified = revision !== savedRevision;
  window.XfaViewerHost?.setModified(modified);
  status.textContent = modified ? 'Unsaved changes' : readOnly ? 'Read only' : '';
}

function edited(event) {
  if (readOnly || !event.target.matches('input, textarea, select')) return;
  revision++;
  reportModified();
}
pages.addEventListener('input', edited);
pages.addEventListener('change', edited);
window.addEventListener('beforeunload', event => {
  if (revision === savedRevision) return;
  event.preventDefault();
  event.returnValue = '';
});

async function save() {
  if (!ready || readOnly) {
    window.XfaViewerHost?.saveFinished(false);
    return;
  }
  if (pendingSave) return;
  pendingSave = { revision, url: null };
  saveButton.disabled = true;
  status.textContent = 'Saving…';
  try {
    const bytes = await pdf.saveDocument();
    pendingSave.url = URL.createObjectURL(new Blob([bytes], { type: 'application/pdf' }));
    const anchor = document.createElement('a');
    anchor.href = pendingSave.url;
    anchor.download = filename.replace(/\.pdf$/i, '') + ' - filled.pdf';
    document.body.append(anchor);
    anchor.click();
    anchor.remove();
  } catch (error) {
    downloadFinished(false);
    status.textContent = 'Unable to save the form: ' + error.message;
  }
}

function downloadFinished(success) {
  if (!pendingSave) return;
  if (success) savedRevision = pendingSave.revision;
  if (pendingSave.url) URL.revokeObjectURL(pendingSave.url);
  pendingSave = null;
  saveButton.disabled = false;
  reportModified();
  window.XfaViewerHost?.saveFinished(success && revision === savedRevision);
  if (!success) status.textContent = 'Save canceled or failed. Changes are still open.';
}

function print() {
  if (!ready || printButton.disabled) return;
  printButton.disabled = true;
  if (window.XfaViewerHost) window.XfaViewerHost.requestPrint();
  else {
    window.print();
    printFinished(true);
  }
}

function printFinished(success) {
  printButton.disabled = !ready;
  if (!success) status.textContent = 'Unable to print the form.';
}
window.XfaViewer = Object.freeze({ save, print, downloadFinished, printFinished });
saveButton.addEventListener('click', save);
printButton.addEventListener('click', print);
window.addEventListener('keydown', event => {
  if (!(event.ctrlKey || event.metaKey) || event.altKey) return;
  if (event.key.toLowerCase() === 's') {
    event.preventDefault();
    save();
  } else if (event.key.toLowerCase() === 'p') {
    event.preventDefault();
    print();
  }
});

try {
  if (source.protocol !== 'file:') throw new Error('Only local PDF forms can be opened.');
  pdf = await getDocument({
    url: source.href,
    enableXfa: true,
    isEvalSupported: false,
    cMapUrl: new URL('pdfjs/cmaps/', import.meta.url).href,
    cMapPacked: true,
    standardFontDataUrl: new URL('pdfjs/standard_fonts/', import.meta.url).href,
    wasmUrl: new URL('pdfjs/wasm/', import.meta.url).href,
  }).promise;
  if (!pdf.isPureXfa) throw new Error('This PDF does not contain a dynamic XFA form.');
  for (let number = 1; number <= pdf.numPages; number++) {
    const page = await pdf.getPage(number);
    const viewport = page.getViewport({ scale: 4 / 3, dontFlip: true });
    const sheet = document.createElement('section');
    sheet.className = 'sheet';
    sheet.setAttribute('aria-label', 'Page ' + number);
    sheet.style.width = viewport.width + 'px';
    sheet.style.height = viewport.height + 'px';
    const layer = document.createElement('div');
    sheet.append(layer);
    pages.append(sheet);
    XfaLayer.render({
      div: layer,
      xfaHtml: await page.getXfa(),
      viewport,
      annotationStorage: pdf.annotationStorage,
      linkService: { addLinkAttributes(anchor) { anchor.removeAttribute('href'); } },
    });
    if (readOnly) {
      sheet.querySelectorAll('input, textarea, select, button').forEach(control => {
        control.disabled = true;
      });
    }
    const printStyle = document.createElement('style');
    printStyle.textContent = '@page xfa' + number + ' { size: ' + viewport.width + 'px ' + viewport.height + 'px; }';
    document.head.append(printStyle);
    sheet.style.page = 'xfa' + number;
  }
  await document.fonts.ready;
  ready = true;
  saveButton.disabled = readOnly;
  printButton.disabled = false;
  reportModified();
} catch (error) {
  status.textContent = 'Unable to open the form: ' + error.message;
}
