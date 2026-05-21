# Tikhoretsk Production DB
Реляционная система управления базами данных с графическим интерфейсом на Python и ядром на C++.

## Состав команды

| Роль | Имя |
|------|-----|
| Тимлид | Чертилина Анастасия |
| Backend-разработчик | Бредихин Александр |
| Frontend-разработчик | Бричков Захар |
| Fullstack-разработчики | Краснова Александра, Головина Анастасия |
| Технический писатель | Краснова Александра |
| Архитектор | Котов Кирилл |

## Структура проекта
Tikhoretsk_production/
├── build/ # Сборка проекта
│ ├── libdb_lib.a # Статическая библиотека C++
│ ├── db_console # Консольный интерфейс
│ └── libdb_python.so # Динамическая библиотека для Python
├── scripts/
│ ├── build_python.sh # Скрипт сборки
│ └── run_python_gui.sh # Скрипт запуска GUI
├── src/
│ ├── core/ # Ядро СУБД
│ │ ├── value.cpp/h # Типы данных
│ │ ├── table.cpp/h # Таблицы и строки
│ │ ├── storage.cpp/h # Сохранение/загрузка (JSON)
│ │ ├── database_metadata.cpp/h # Метаданные базы данных
│ │ └── catalog.cpp/h # Управление базами данных
│ ├── sql/ # Обработка SQL
│ │ ├── ast.cpp/h # AST-дерево запроса
│ │ ├── parser.cpp/h # Парсер SQL
│ │ ├── executor.cpp/h # Исполнитель запросов
│ │ └── result.cpp/h # Формат результата
│ ├── client/ # Публичный API
│ │ ├── database.cpp/h # Главный класс Database
│ │ ├── result.cpp/h # Класс Result
│ │ └── c_api.cpp # C-интерфейс для Python
│ └── console/ # Консольный интерфейс
├── python/ # Python GUI
│ ├── gui_beautiful.py # Графический интерфейс
│ └── db_wrapper.py # Python-обёртка
└── data/ # Хранилище баз данных

text

## Архитектура

Проект построен по многоуровневому принципу:

1. **Python GUI** (tkinter + ctypes) - пользовательский интерфейс
2. **C API** (c_api.cpp) - мост между Python и C++
3. **Public API** (client/) - основной интерфейс для клиентов
4. **SQL Layer** (sql/) - парсер и исполнитель запросов
5. **Core** (core/) - хранение и управление данными
6. **Файловое хранилище** (data/) - постоянное сохранение в JSON

## Паттерны проектирования

| Паттерн | Место применения | Обоснование |
|---------|------------------|-------------|
| Pimpl | class Database | Скрытие реализации, ускорение компиляции |
| Facade | class Database | Единый простой интерфейс к сложной подсистеме |
| Builder | Parser | Поэтапное построение AST-дерева запроса |

## Соответствие MVP-требованиям

| Требование | Реализация |
|------------|------------|
| Постоянное хранилище | Данные сохраняются в JSON-файлах в папке data/ |
| C++ библиотека | Статическая (libdb_lib.a) и динамическая (libdb_python.so) |
| Паттерны проектирования | Pimpl, Facade, Builder |
| Скрипты сборки | scripts/build_python.sh |
| Консольный интерфейс | src/console/main.cpp |
| Графический интерфейс | python/gui_beautiful.py |

## Поддерживаемые SQL-команды

### DDL (Data Definition Language)

| Команда | Описание | Пример |
|---------|----------|--------|
| CREATE DATABASE | Создание базы данных | `CREATE DATABASE test;` |
| DROP DATABASE | Удаление базы данных | `DROP DATABASE test;` |
| CREATE TABLE | Создание таблицы | `CREATE TABLE users (id INT, name TEXT);` |
| DROP TABLE | Удаление таблицы | `DROP TABLE users;` |

### DML (Data Manipulation Language)

| Команда | Описание | Пример |
|---------|----------|--------|
| INSERT | Вставка строк | `INSERT INTO users VALUES (1, 'Alice');` |
| SELECT | Выборка данных | `SELECT * FROM users;` |
| UPDATE | Обновление данных | `UPDATE users SET name = 'Bob' WHERE id = 1;` |
| DELETE | Удаление строк | `DELETE FROM users WHERE id = 1;` |

### Поддерживаемые типы данных

- INT - целые числа
- TEXT - строки
- FLOAT - числа с плавающей точкой
- BOOL - логические значения (true/false)

### Поддерживаемые операторы в WHERE

- `=` - равно
- `>` - больше
- `<` - меньше
- `>=` - больше или равно
- `<=` - меньше или равно
- `AND` - логическое И
- `OR` - логическое ИЛИ

## Примеры выполнения запросов

| Запрос | Ожидаемый результат |
|--------|---------------------|
| `CREATE DATABASE test;` | 0 row(s) affected (база автоматически становится текущей) |
| `CREATE TABLE users (id INT, name TEXT);` | 0 row(s) affected |
| `INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');` | 2 row(s) affected |
| `SELECT * FROM users;` | Таблица с двумя строками, колонки id и name |
| `SELECT name FROM users WHERE id > 1;` | Одна строка: Bob |
| `UPDATE users SET name = 'Alicia' WHERE id = 1;` | 1 row(s) affected |
| `SELECT * FROM users;` | Первая строка: 1, Alicia; вторая: 2, Bob |
| `DELETE FROM users WHERE id = 2;` | 1 row(s) affected |
| `SELECT * FROM users;` | Только одна строка: 1, Alicia |
| `DROP TABLE users;` | 0 row(s) affected |
| `DROP DATABASE test;` | 0 row(s) affected |

## Установка и запуск

### Требования

- Операционная система: Linux (Ubuntu 20.04+), macOS, Windows (WSL)
- Компилятор: GCC 9+ или Clang 10+
- Python: 3.8+
- CMake: 3.10+

### Сборка

```bash
# Клонирование репозитория
git clone https://github.com/achertilina/Tikhoretsk_production.git
cd Tikhoretsk_production

# Установка зависимостей (Ubuntu/Debian)
sudo apt-get install -y cmake g++ python3 python3-tk nlohmann-json3-dev

# Сборка проекта
chmod +x scripts/build_python.sh
./scripts/build_python.sh
Запуск
bash
# Запуск Python GUI
chmod +x run_gui.sh
./run_gui.sh

# Или напрямую
cd python
python3 gui_beautiful.py
Интерфейс
Панель подключения
Data Path - путь к папке с базами данных

Open Database - открыть хранилище

Database - выбор активной базы данных

Create Database - создание новой базы данных

Drop Database - удаление выбранной базы данных

Refresh - обновить список баз данных

Редактор SQL
Подсветка синтаксиса (ключевые слова, строки, числа)

Примеры запросов по кнопкам

Результаты
SELECT - отображается в виде таблицы

INSERT/UPDATE/DELETE - показывается количество затронутых строк

Горячие клавиши
F5 - выполнить запрос

Ctrl+Enter - выполнить запрос

Ctrl+D - очистить редактор
