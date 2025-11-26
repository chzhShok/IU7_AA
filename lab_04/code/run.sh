# Создаем build директорию и генерируем проект
cmake -S . -B build

# Собираем программу
cmake --build build -j
