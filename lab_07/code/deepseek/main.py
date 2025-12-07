import os
import re
from collections import Counter
from typing import Dict, List, Tuple

import PyPDF2


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


def _extract_labels(pdf_path: str) -> List[Dict]:
    items: List[Dict] = []

    with open(pdf_path, "rb") as file:
        reader = PyPDF2.PdfReader(file)

        for page_index, page in enumerate(reader.pages, start=1):
            text = page.extract_text() or ""
            lines = text.splitlines()

            for line_index, line in enumerate(lines, start=1):
                for match in LABEL_REGEX.finditer(line):
                    keyword = match.group("keyword")
                    number = match.group("number")
                    raw = match.group(0).strip()

                    kind = _normalize_kind(keyword)
                    scheme = _detect_scheme(number)

                    items.append(
                        {
                            "raw_label": raw,
                            "kind": kind,
                            "scheme": scheme,
                            "number": number,
                            "page": page_index,
                            "line": line_index,
                        }
                    )

    return items

def check_numbering_mixing(pdf_path: str) -> Tuple[bool, List[Dict]]:
    try:
        labels = _extract_labels(pdf_path)
    except Exception as e:
        print(f"Error while processing file {pdf_path}: {e}")
        return False, []

    if not labels:
        return False, []

    scheme_counter = Counter(label["scheme"] for label in labels)
    if len(scheme_counter) <= 1:
        elements_info: List[Dict] = []
        for lbl in labels:
            elements_info.append(
                {
                    "original_name": lbl["raw_label"],
                    "type": lbl["kind"],
                    "number": lbl["number"],
                    "numbering_type": lbl["scheme"],
                    "page": lbl["page"],
                    "line": lbl["line"],
                    "description": lbl["raw_label"],
                }
            )
        return False, elements_info

    dominant_scheme, _ = scheme_counter.most_common(1)[0]
    offending_schemes = {s for s in scheme_counter.keys() if s != dominant_scheme}

    has_mixing = False
    elements_info: List[Dict] = []

    for lbl in labels:
        is_offending = lbl["scheme"] in offending_schemes
        if is_offending:
            has_mixing = True

        info = {
            "original_name": lbl["raw_label"],
            "type": lbl["kind"],
            "number": lbl["number"],
            "numbering_type": lbl["scheme"],
            "page": lbl["page"],
            "line": lbl["line"],
            "description": lbl["raw_label"],
            "is_offending": is_offending,
        }
        elements_info.append(info)

    if has_mixing:
        mixing_details = [
            "Detected mixed numbering schemes for illustration elements.",
            f"Dominant scheme: {dominant_scheme}",
            "Offending schemes: " + ", ".join(sorted(offending_schemes)),
        ]
        for info in elements_info:
            info["mixing_details"] = mixing_details

    return has_mixing, elements_info

def main():
    import sys
    import shutil
    
    if len(sys.argv) < 2:
        print("Usage: python check_numbering.py <path_to_pdf_file_or_directory>")
        return
    
    path = sys.argv[1]
    
    pdf_files = []
    if os.path.isfile(path) and path.lower().endswith('.pdf'):
        pdf_files.append(path)
    elif os.path.isdir(path):
        for file in os.listdir(path):
            if file.lower().endswith('.pdf'):
                pdf_files.append(os.path.join(path, file))
    else:
        print(f"Путь {path} не является PDF-файлом или папкой")
        return
    
    if not pdf_files:
        print("No PDF files found")
        return
    
    print("=" * 80)
    print("Checking for mixed numbering types of tables and figures")
    print("=" * 80)
    
    results = []
    files_with_errors = []
    
    for pdf_file in pdf_files:
        filename = os.path.basename(pdf_file)
        print(f"\nAnalyzing file: {filename}")
        
        has_mixing, elements_info = check_numbering_mixing(pdf_file)
        
        file_result = {
            'file': filename,
            'has_mixing': has_mixing,
            'elements_info': elements_info,
            'elements_count': len(elements_info)
        }
        results.append(file_result)
        
        if not elements_info:
            print("  Result: OK - tables and figures not found")
        elif not has_mixing:
            print(f"  Result: OK - numbering is consistent ({len(elements_info)} elements)")
            if len(elements_info) > 0:
                print(f"  Numbering type: {elements_info[0]['numbering_type']}")
        else:
            print("  Result: VIOLATION - mixed numbering types detected")
            files_with_errors.append(pdf_file)
    
    print("\n" + "=" * 80)
    print("SUMMARY TABLE OF RESULTS")
    print("=" * 80)
    print(f"{'File name':<40} | {'Violation (mixed)':<20} | {'Elements count':<15}")
    print("-" * 85)
    
    for result in results:
        filename = result["file"]
        has_mixing = "YES" if result["has_mixing"] else "NO"
        count = result['elements_count']
        
        print(f"{filename:<40} | {has_mixing:<20} | {count:<15}")
    
    if files_with_errors:
        print(f"\n{'='*80}")
        print(f"DETAILED INFORMATION FOR FILES WITH ERRORS ({len(files_with_errors)}):")
        print(f"{'='*80}")
        
        error_dir = "files_with_errors"
        if not os.path.exists(error_dir):
            os.makedirs(error_dir)
        
        for i, file_path in enumerate(files_with_errors, 1):
            filename = os.path.basename(file_path)
            dst_path = os.path.join(error_dir, filename)
            
            try:
                shutil.copy2(file_path, dst_path)
                print(f"\n{i}. {filename} (copied to {error_dir}/):")
                
                has_mixing, elements_info = check_numbering_mixing(file_path)
                
                tables = [e for e in elements_info if e['type'] == 'table']
                figures = [e for e in elements_info if e['type'] == 'figure']
                
                print(f"   Detected items ({len(elements_info)}):")
                
                if tables:
                    print(f"   Tables ({len(tables)}):")
                    for element in tables:
                        print(f"     • {element['description']} - {element['numbering_type']} ")
                
                if figures:
                    print(f"   Figures ({len(figures)}):")
                    for element in figures:
                        print(f"     • {element['description']} - {element['numbering_type']} ")
                
                if elements_info and 'mixing_details' in elements_info[0]:
                    print("   Issues:")
                    for detail in elements_info[0]['mixing_details']:
                        print(f"     • {detail}")
                        
            except Exception as e:
                print(f"\n{i}. {filename} - error while copying: {e}")
        
        print(f"\nAll files with errors have been saved to '{error_dir}/'")
    else:
        print("\nNo files with numbering violations were found.")
    
    print(f"\n{'='*80}")
    print("SUMMARY STATISTICS")
    print(f"{'='*80}")
    total_files = len(results)
    error_files = len(files_with_errors)
    ok_files = total_files - error_files
    total_elements = sum(r['elements_count'] for r in results)
    
    print(f"Total processed files: {total_files}")
    print(f"  - Without errors: {ok_files}")
    print(f"  - With errors: {error_files}")
    print(f"  - Total elements (tables/figures): {total_elements}")

if __name__ == "__main__":
    main()
