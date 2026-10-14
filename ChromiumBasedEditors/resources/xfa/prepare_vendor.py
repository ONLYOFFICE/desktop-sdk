#!/usr/bin/env python3

import argparse
import base64
import hashlib
import io
from pathlib import Path, PurePosixPath
import shutil
import tarfile
import tempfile
import urllib.request


VERSION = "5.6.205"
URL = "https://registry.npmjs.org/pdfjs-dist/-/pdfjs-dist-" + VERSION + ".tgz"
SHA512 = "tlUj+2IDa7G1SbvBNN74UHRLJybZDWYom+k6p5KIZl7huBvsA4APi6mKL+zCxd3tLjN5hOOEE9Tv7VdzO88pfg=="
FILES = {"LICENSE", "legacy/build/pdf.mjs", "legacy/build/pdf.worker.mjs", "web/pdf_viewer.css"}
DIRECTORIES = {"cmaps", "standard_fonts", "wasm"}


def install(archive, output):
    if base64.b64encode(hashlib.sha512(archive).digest()).decode() != SHA512:
        raise ValueError("PDF.js archive checksum does not match version " + VERSION)

    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".pdfjs-", dir=output.parent) as temporary:
        stage = Path(temporary) / "pdfjs"
        stage.mkdir()
        extracted = set()
        with tarfile.open(fileobj=io.BytesIO(archive), mode="r:gz") as package:
            for member in package.getmembers():
                path = PurePosixPath(member.name)
                if not member.isfile() or path.parts[0] != "package" or ".." in path.parts:
                    continue
                relative = PurePosixPath(*path.parts[1:])
                if str(relative) not in FILES and relative.parts[0] not in DIRECTORIES:
                    continue
                destination = stage.joinpath(*relative.parts)
                destination.parent.mkdir(parents=True, exist_ok=True)
                with package.extractfile(member) as source, destination.open("wb") as target:
                    shutil.copyfileobj(source, target)
                extracted.add(str(relative))
        if not FILES.issubset(extracted):
            raise ValueError("PDF.js archive is missing required files")
        (stage / "version.txt").write_text(VERSION + "\n", encoding="utf-8")
        if output.exists():
            shutil.rmtree(output)
        stage.rename(output)


def main():
    parser = argparse.ArgumentParser(description="Prepare the pinned PDF.js runtime for the XFA viewer.")
    parser.add_argument("--archive", type=Path, help="Use a downloaded npm archive instead of downloading it")
    parser.add_argument("--output", type=Path, default=Path(__file__).resolve().parent / "pdfjs")
    args = parser.parse_args()
    if args.archive:
        archive = args.archive.read_bytes()
    else:
        with urllib.request.urlopen(URL, timeout=60) as response:
            archive = response.read()
    install(archive, args.output.resolve())
    print("Prepared PDF.js " + VERSION + " in " + str(args.output))


if __name__ == "__main__":
    main()
