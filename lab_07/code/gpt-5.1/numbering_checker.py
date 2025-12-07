from __future__ import annotations

from dataclasses import dataclass
from collections import Counter
from typing import Iterable, List, Tuple
import re

from PyPDF2 import PdfReader  


@dataclass
class MatchLocation:
    page: int  
    line: int  


@dataclass
class NumberingMatch:
    raw_label: str
    kind: str
    scheme: str
    location: MatchLocation


ILLUSTRATION_KEYWORDS = [
    r"рис\.?",  
    r"рисунок",
    r"табл\.?",  
    r"таблица",
    r"fig\.?",  
    r"figure",
    r"table",
]

LABEL_REGEX = re.compile(
    rf"""
    (?P<keyword>{'|'.join(ILLUSTRATION_KEYWORDS)})  
    \s*                                            
    (?P<number>\d+(?:\.\d+)*)                      
    """,
    re.IGNORECASE | re.VERBOSE,
)


def _normalize_kind(keyword: str) -> str:
    kw = keyword.lower()
    if kw.startswith(("рис", "fig")):
        return "figure"
    if kw.startswith(("таб", "table")):
        return "table"
    return "other"


def _detect_scheme(number: str) -> str:
    return "sectional" if "." in number else "global"


def iter_labels_in_page_text(page_text: str, page_number: int) -> Iterable[NumberingMatch]:
    if not page_text:
        return []

    matches: List[NumberingMatch] = []

    lines = page_text.splitlines()
    for line_index, line in enumerate(lines, start=1):
        for m in LABEL_REGEX.finditer(line):
            keyword = m.group("keyword")
            number = m.group("number")
            kind = _normalize_kind(keyword)
            scheme = _detect_scheme(number)
            raw_label = m.group(0)

            matches.append(
                NumberingMatch(
                    raw_label=raw_label.strip(),
                    kind=kind,
                    scheme=scheme,
                    location=MatchLocation(page=page_number, line=line_index),
                )
            )

    return matches


def extract_numbering_matches(pdf_path: str) -> List[NumberingMatch]:
    reader = PdfReader(pdf_path)
    all_matches: List[NumberingMatch] = []

    for page_number, page in enumerate(reader.pages, start=1):
        text = page.extract_text() or ""
        page_matches = list(iter_labels_in_page_text(text, page_number))
        all_matches.extend(page_matches)

    return all_matches


def search_mixed_numbering_in_pdf(
    pdf_path: str,
) -> Tuple[bool, List[Tuple[str, Tuple[int, int]]]]:
    all_matches = extract_numbering_matches(pdf_path)
    if not all_matches:
        return False, []

    scheme_counter = Counter(m.scheme for m in all_matches)
    if len(scheme_counter) <= 1:
        return False, []

    dominant_scheme, _ = scheme_counter.most_common(1)[0]
    offending_schemes = {s for s in scheme_counter.keys() if s != dominant_scheme}

    offending_matches: List[Tuple[str, Tuple[int, int]]] = []
    for m in all_matches:
        if m.scheme in offending_schemes:
            loc = m.location
            offending_matches.append((m.raw_label, (loc.page, loc.line)))

    has_mixed = bool(offending_matches)
    return has_mixed, offending_matches


__all__ = [
    "MatchLocation",
    "NumberingMatch",
    "extract_numbering_matches",
    "search_mixed_numbering_in_pdf",
]
