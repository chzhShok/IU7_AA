# test_pdf_generator.py
from reportlab.lib.pagesizes import letter
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
import os

# Регистрируем шрифт с поддержкой кириллицы
def register_cyrillic_font():
    try:
        # Пытаемся использовать стандартные шрифты
        pdfmetrics.registerFont(TTFont('Arial', 'arial.ttf'))
        return 'Arial'
    except:
        try:
            # Альтернативный вариант
            pdfmetrics.registerFont(TTFont('DejaVuSans', 'DejaVuSans.ttf'))
            return 'DejaVuSans'
        except:
            # Если шрифты не найдены, используем встроенный стандартный
            return 'Helvetica'

def create_test_pdf(filename, content_lines):
    c = canvas.Canvas(filename, pagesize=letter)
    width, height = letter
    
    # Регистрируем и получаем шрифт
    font_name = register_cyrillic_font()
    
    y_position = height - 50
    line_height = 20
    
    for line in content_lines:
        # Устанавливаем шрифт с поддержкой кириллицы
        c.setFont(font_name, 12)
        c.drawString(50, y_position, line)
        y_position -= line_height
        
        if y_position < 50:
            c.showPage()
            y_position = height - 50
    
    c.save()
    print(f"Создан файл: {filename}")

# Создаем тестовые файлы с разными вариантами написания
test_files = [
    # 1. Русские названия, консистентная сквозная нумерация
    ("test_russian_through.pdf", [
        "Научная работа на русском",
        "Таблица 1: Основные параметры",
        "таблица 2: Дополнительные данные",
        "Рисунок 1: Структурная схема",
        "рисунок 2: График результатов"
    ]),
    
    # 2. Русские названия, консистентная пораздельная нумерация
    ("test_russian_sectional.pdf", [
        "Диссертация на русском",
        "Глава 1. Теория",
        "Таблица 1.1: Классификация методов",
        "таблица 1.2: Сравнение подходов",
        "Рисунок 1.1: Блок-схема алгоритма",
        "рисунок 1.2: Диаграмма состояний"
    ]),
    
    # 3. Английские названия, консистентная сквозная нумерация
    ("test_english_through.pdf", [
        "Scientific paper in English",
        "Table 1: Main parameters",
        "table 2: Additional data",
        "Figure 1: Structure diagram",
        "figure 2: Results chart"
    ]),
    
    # 4. Английские названия, консистентная пораздельная нумерация
    ("test_english_sectional.pdf", [
        "Technical documentation in English",
        "Chapter 1. Introduction",
        "Table 1.1: System parameters",
        "table 1.2: Component characteristics",
        "Figure 1.1: Architecture overview",
        "figure 1.2: Performance metrics"
    ]),
    
    # 5. Смешанные названия (русские + английские), консистентная нумерация
    ("test_mixed_names_consistent.pdf", [
        "Международная публикация",
        "Table 1: International standards",  # английский
        "Таблица 2: Локальные данные",      # русский
        "Figure 1: Global trends",          # английский
        "Рисунок 2: Региональные особенности"  # русский
    ]),
    
    # 6. Смешение типов нумерации (русские названия)
    ("test_mixed_numbering_russian.pdf", [
        "Отчет с ошибками (русский)",
        "Таблица 1: Общие сведения",         # сквозная
        "Таблица 2.1: Детальный анализ",     # пораздельная - ОШИБКА!
        "Рисунок 1.1: Схема процесса",      # пораздельная
        "Рисунок 2: Итоговый график"        # сквозная - ОШИБКА!
    ]),
    
    # 7. Смешение типов нумерации (английские названия)
    ("test_mixed_numbering_english.pdf", [
        "Report with errors (English)",
        "Table 1: General information",      # сквозная
        "Table 2.1: Detailed analysis",      # пораздельная - ОШИБКА!
        "Figure 1.1: Process diagram",       # пораздельная
        "Figure 2: Final chart"              # сквозная - ОШИБКА!
    ]),
    
    # 8. Смешение между таблицами и рисунками
    ("test_mixed_between_types.pdf", [
        "Сложный случай смешения",
        "Таблица 1: Исходные данные",        # таблицы - сквозная
        "Table 2: Расчетные показатели",     # таблицы - сквозная
        "Figure 1.1: Visualization",         # рисунки - пораздельная - ОШИБКА!
        "Рисунок 2.1: Analysis results"      # рисунки - пораздельная - ОШИБКА!
    ]),
    
    # 9. Только русские таблицы с разной нумерацией
    ("test_russian_tables_mixed.pdf", [
        "Документ с таблицами",
        "Таблица 1: Первая таблица",         # сквозная
        "таблица 2.1: Вторая таблица",       # пораздельная - ОШИБКА!
        "Таблица 3: Третья таблица"          # сквозная - ОШИБКА!
    ]),
    
    # 10. Только английские рисунки с разной нумерацией
    ("test_english_figures_mixed.pdf", [
        "Document with figures only",
        "Figure 1: First figure",            # сквозная
        "figure 2.1: Second figure",         # пораздельная - ОШИБКА!
        "Figure 3: Third figure"             # сквозная - ОШИБКА!
    ])
]

# Создаем тестовую директорию
test_dir = "test_pdfs"
if not os.path.exists(test_dir):
    os.makedirs(test_dir)

# Генерируем тестовые файлы
for filename, content in test_files:
    filepath = os.path.join(test_dir, filename)
    create_test_pdf(filepath, content)

print(f"\nВсе тестовые файлы созданы в папке '{test_dir}'")
print("\nОжидаемые результаты:")
print("1. test_russian_through.pdf - OK (сквозная нумерация)")
print("2. test_russian_sectional.pdf - OK (пораздельная нумерация)")
print("3. test_english_through.pdf - OK (сквозная нумерация)")
print("4. test_english_sectional.pdf - OK (пораздельная нумерация)")
print("5. test_mixed_names_consistent.pdf - OK (сквозная нумерация)")
print("6. test_mixed_numbering_russian.pdf - НАРУШЕНИЕ (смешение типов)")
print("7. test_mixed_numbering_english.pdf - НАРУШЕНИЕ (смешение типов)")
print("8. test_mixed_between_types.pdf - НАРУШЕНИЕ (таблицы и рисунки разные)")
print("9. test_russian_tables_mixed.pdf - НАРУШЕНИЕ (таблицы разные)")
print("10. test_english_figures_mixed.pdf - НАРУШЕНИЕ (рисунки разные)")
print("\nДля тестирования запустите: python check_numbering.py test_pdfs/")