from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Iterable, List, NoReturn

from numbering_checker import search_mixed_numbering_in_pdf


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Check PDF documents for mixed illustration numbering schemes.\n"
            "Allowed schemes are either only global (1, 2, 3, ...) or only "
            "sectional (1.1, 1.2, ..., 2.1, 2.2, ...)."
        )
    )
    parser.add_argument(
        "path",
        help="Path to a PDF file or to a directory containing PDF files.",
    )
    return parser.parse_args(argv)


def _collect_pdf_paths(path: str) -> List[Path]:
    target = Path(path)
    if not target.exists():
        raise FileNotFoundError(path)

    if target.is_file():
        if target.suffix.lower() != ".pdf":
            raise ValueError(f"Not a PDF file: {target}")
        return [target]

    if target.is_dir():
        pdfs = sorted(
            p for p in target.iterdir()
            if p.is_file() and p.suffix.lower() == ".pdf"
        )
        return pdfs

    raise ValueError(f"Path is neither file nor directory: {target}")


def _process_single_pdf(pdf_path: Path) -> int:
    print(f"=== {pdf_path} ===")

    try:
        has_mixed, offending = search_mixed_numbering_in_pdf(str(pdf_path))
    except FileNotFoundError:
        print(f"Error: file not found: {pdf_path}", file=sys.stderr)
        return 1
    except Exception as exc:  
        print(f"Error while processing PDF {pdf_path}: {exc}", file=sys.stderr)
        return 1

    if not has_mixed:
        print("Mixed numbering: NO")
        return 0

    print("Mixed numbering: YES")
    print("Offending labels (minority schemes):")
    for label, (page, line) in offending:
        print(f"  page {page}, line {line}: {label}")

    return 0


def main(argv: list[str] | None = None) -> NoReturn:
    args = parse_args(argv)

    try:
        pdf_paths = _collect_pdf_paths(args.path)
    except (FileNotFoundError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        raise SystemExit(1)

    if not pdf_paths:
        print(f"No PDF files found for path: {args.path}", file=sys.stderr)
        raise SystemExit(1)

    exit_code = 0
    for pdf_path in pdf_paths:
        if len(pdf_paths) > 1:
            print()
        result = _process_single_pdf(pdf_path)
        if result != 0:
            exit_code = result

    raise SystemExit(exit_code)


if __name__ == "__main__":
    main()
