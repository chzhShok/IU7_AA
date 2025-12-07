import sys
import os
import re
import PyPDF2
from typing import List, Tuple

def check_mixed_numbering(pdf_path: str) -> Tuple[bool, List[Tuple[str, int, int]]]:
    sequential_regex = re.compile(r'(?:Рисунок|Таблица|Figure|Table)\s+\d+(?!\.)\b', re.IGNORECASE)
    section_regex = re.compile(r'(?:Рисунок|Таблица|Figure|Table)\s+\d+(\.\d+)+', re.IGNORECASE)

    sequential_matches = []
    section_matches = []

    try:
        with open(pdf_path, 'rb') as file:
            reader = PyPDF2.PdfReader(file)
            for page_num, page in enumerate(reader.pages):
                text = page.extract_text()
                
                if not text:
                    continue
                
                lines = text.split('\n')
                for line_num, line in enumerate(lines):
                    for match in sequential_regex.finditer(line):
                        sequential_matches.append((match.group(0), page_num + 1, line_num + 1))

                    for match in section_regex.finditer(line):
                        section_matches.append((match.group(0), page_num + 1, line_num + 1))

    except FileNotFoundError:
        print(f"Error: file not found: {pdf_path}")
        return False, []
    except Exception as e:
        print(f"Error while processing file {pdf_path}: {e}")
        return False, []

    is_mixed = bool(sequential_matches and section_matches)
    all_matches = sequential_matches + section_matches if is_mixed else []
    all_matches.sort(key=lambda x: (x[1], x[2]))

    return is_mixed, all_matches

def print_results(pdf_file_path: str, is_mixed: bool, findings: List[Tuple[str, int, int]]):
    print("\n" + "="*50)
    print(f"      Analysis results for file: {os.path.basename(pdf_file_path)}")
    print("="*50)
    print(f"Full path: {pdf_file_path}")
    print(f"Mixed numbering detected: {'Yes' if is_mixed else 'No'}")

    if is_mixed and findings:
        first_finding = findings[0]
        print(f"Coordinates of first finding: Page {first_finding[1]}, line {first_finding[2]}")
        print("\nAll found instances:")
        for find in findings:
            print(f"  - '{find[0]}' (Page {find[1]}, line {find[2]})")
    else:
        print("Coordinates of first finding: -")
    print("="*50)

def main():
    if len(sys.argv) < 2:
        print("Usage: python main.py <path_to_PDF_file_or_directory>")
        sys.exit(1)
        
    input_path = sys.argv[1]

    if not os.path.exists(input_path):
        print(f"Error: The specified path does not exist: {input_path}")
        sys.exit(1)

    pdf_files_to_process = []
    if os.path.isdir(input_path):
        print(f"Analysis of directory: {input_path}")
        pdf_files_to_process.extend(
            [os.path.join(input_path, f) for f in os.listdir(input_path) if f.lower().endswith('.pdf')]
        )
        if not pdf_files_to_process:
            print("No PDF files found in the specified directory.")
            sys.exit(0)
    elif os.path.isfile(input_path):
        if input_path.lower().endswith('.pdf'):
            pdf_files_to_process.append(input_path)
        else:
            print("Error: The specified file is not a PDF file.")
            sys.exit(1)
    
    print(f"\nFound for processing: {len(pdf_files_to_process)} PDF file(s).")
    
    for pdf_file_path in pdf_files_to_process:
        is_mixed, findings = check_mixed_numbering(pdf_file_path)
        print_results(pdf_file_path, is_mixed, findings)

if __name__ == "__main__":
    main()
