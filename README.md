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
```
project/
├── build/                              # Создаётся скриптами сборки
│   ├── libdb.a                         # Статическая библиотека (ядро + API)
│   ├── db_console                      # Консольный интерфейс
│   └── db_gui                          # Графический интерфейс (опционально)
│
├── scripts/
│   ├── build.sh                        # Установка зависимостей + сборка проекта
│   └── run.sh                          # Запуск интерфейса (консоль/GUI)
│
├── src/
│   │
│   ├── core/                         
│   │   │
│   │   ├── value.h/cpp
│   │   │   # Определяет допустимые типы данных (INT, FLOAT, TEXT, BOOL, VARCHAR)
│   │   │   # Класс Value хранит тип + значение (int/float/string/bool/null)
│   │   │   # Умеет сравнивать значения (==, <, >) — нужно для WHERE
│   │   │   # Умеет преобразовывать Value в строку и из строки (для сериализации)
│   │   │
│   │   ├── table.h/cpp
│   │   │   # Хранит схему таблицы: имя колонки → тип данных
│   │   │   # Хранит строки данных: vector<vector<Value>>
│   │   │   # Методы: insert_row(), update_row(), delete_row()
│   │   │   # Метод filter(condition) возвращает индексы строк, подходящих под условие
│   │   │   # Метод project(column_indices) возвращает новую таблицу с выбранными колонками
│   │   │   # Отслеживает, была ли таблица изменена (флаг dirty для сохранения)
│   │   │
│   │   ├── storage.h/cpp
│   │   │   # Сохраняет ОДНУ таблицу в файл (формат: JSON)
│   │   │   # Загружает таблицу из файла
│   │   │   # Не знает о базах данных — только имя файла + Table
│   │   │   # Методы: save(table, filepath), load(filepath) -> Table
│   │   │
│   │   ├── database_metadata.h/cpp
│   │   │   # Представляет одну базу данных
│   │   │   # Хранит: имя базы, список имён таблиц в ней
│   │   │   # Знает путь к своей папке (например, "data/mydb/")
│   │   │   # Умеет сохранять/загружать .metadata.json файл
│   │   │   # Не хранит сами таблицы — только их имена
│   │   │
│   │   └── catalog.h/cpp
│   │       # Главный менеджер всех данных на диске
│   │       # Хранит корневую папку (например, "data/")
│   │       # При загрузке: сканирует подпапки → читает .metadata.json → через Storage загружает все таблицы
│   │       # Хранит загруженные таблицы: map<имя_базы, map<имя_таблицы, Table>>
│   │       # Предоставляет API для Executor: get_table(db, table), create_table(), drop_table()
│   │       # Предоставляет API для баз данных: create_database(), drop_database()
│   │       # При close() проходит по изменённым таблицам и вызывает Storage::save()
│   │
│   ├── sql/                           # ===== ОБРАБОТКА SQL =====
│   │   │
│   │   ├── ast.h/cpp
│   │   │   # Структуры данных для представления SQL-запроса в памяти
│   │   │   # Содержит: SelectQuery, InsertQuery, UpdateQuery, DeleteQuery
│   │   │   # Содержит: CreateTableQuery, DropTableQuery
│   │   │   # Содержит: CreateDatabaseQuery, DropDatabaseQuery
│   │   │   # Содержит: Condition (для WHERE с AND/OR/NOT и сравнениями)
│   │   │   # AST — результат работы парсера, вход для исполнителя
│   │   │
│   │   ├── parser.h/cpp
│   │   │   # Принимает строку SQL ("SELECT * FROM users WHERE age > 18")
│   │   │   # Разбивает на токены (лексер)
│   │   │   # Проверяет синтаксис по грамматике
│   │   │   # Строит и возвращает AST (например, SelectQuery с заполненными полями)
│   │   │   # При ошибке бросает исключение с указанием места и причины
│   │   │
│   │   ├── executor.h/cpp
│   │   │   # Принимает AST и указатель на Catalog
│   │   │   # Выполняет запрос, используя API Catalog и Table
│   │   │   # Для SELECT: вызывает Catalog::get_table() → filter() → project()
│   │   │   # Для INSERT: вызывает Table::insert_row() → помечает таблицу как dirty
│   │   │   # Для UPDATE/DELETE: filter() → update_row()/delete_row()
│   │   │   # Для DDL (CREATE/DROP): вызывает Catalog::create_table() и т.д.
│   │   │   # Возвращает QueryResult (таблицу или количество изменённых строк)
│   │   │
│   │   └── result.h/cpp
│   │       # Внутренний формат результата выполнения запроса
│   │       # Может быть: SELECT → таблица (имена колонок + vector<vector<Value>>)
│   │       # Может быть: INSERT/UPDATE/DELETE → количество затронутых строк
│   │       # Может быть: ошибка → сообщение
│   │       # Используется для передачи от Executor к клиенту (через client/result.h)
│   │
│   ├── client/                       # ===== ПУБЛИЧНЫЙ API =====
│   │   │
│   │   ├── database.h/cpp
│   │   │   # ГЛАВНЫЙ КЛАСС — пользователь включает только этот файл
│   │   │   # Метод open(root_path) → создаёт Catalog и загружает все базы
│   │   │   # Метод execute(sql) → передаёт строку в Parser → Executor
│   │   │   # Метод close() → вызывает Catalog::save_all()
│   │   │   # Единственный класс, который экспортируется из библиотеки
│   │   │
│   │   └── result.h/cpp
│   │       # Публичная версия результата (обёртка над sql/result.h)
│   │       # Скрывает от пользователя класс Value (работает через строки)
│   │       # Методы: is_select(), get_column_names(), get_row_as_strings()
│   │       # Методы: get_affected_rows(), get_error_message()
│   │       # Метод success() — успешно ли выполнен запрос
│   │
│   ├── console/                      # ===== КОНСОЛЬНЫЙ ИНТЕРФЕЙС =====
│   │   │
│   │   └── main.cpp
│   │       # Точка входа для консольной версии
│   │
│   └── gui/                          # ===== ГРАФИЧЕСКИЙ ИНТЕРФЕЙС (опционально) =====
│       │
│       ├── main.cpp
│       │   # Точка входа для GUI-версии
│       │   # Создаёт QApplication → создаёт MainWindow → запускает событийный цикл
│       │
│       ├── main_window.h/cpp
│       │   # Главное окно приложения
│       │   # Содержит: меню, панель инструментов, центральный виджет с разделителем
│       │   # Слева/сверху: QueryEditor, справа/снизу: ResultsTable
│       │   # Внизу: статусная строка (текущая БД, результат операции)
│       │   # Сигналы: on_execute_clicked(), on_db_changed()
│       │
│       ├── query_editor.h/cpp
│       │   # Многострочное текстовое поле для ввода SQL
│       │   # Подсветка синтаксиса SQL (ключевые слова: SELECT, FROM, WHERE...)
│       │   # Номера строк
│       │   # История запросов (Ctrl+Up/Ctrl+Down)
│       │   # Горячие клавиши: F5 — выполнить, Ctrl+Enter — выполнить
│       │
│       ├── results_table.h/cpp
│       │   # Отображает результат SELECT в виде таблицы
│       │   # Заголовки колонок — имена из Result::get_column_names()
│       │   # Данные строк — Result::get_row_as_strings()
│       │   # Поддержка сортировки по клику на заголовок
│       │   # Поддержка копирования ячейки/строки в буфер обмена
│       │
│       └── connection_panel.h/cpp
│           # Панель управления подключением к данным
│           # Кнопка "Открыть базу" → диалог выбора папки
│           # Выпадающий список существующих баз данных
│           # Кнопки "Создать БД", "Удалить БД"
│           # Поле ввода имени новой таблицы
│           # Текущий путь к корневой папке
│
├── CMakeLists.txt                     # Конфигурация сборки
│                                     # Определяет: библиотеку db_lib, консольную версию db_console
│                                     # Опционально: GUI-версию db_gui (если BUILD_GUI=ON)
│
├── README.md                          # Инструкции по сборке и запуску
│
└── .gitignore                         # Игнорировать build/, *.o, *.a, .idea/
```
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

#Или напрямую
cd python
python3 gui_beautiful.py
```
## Интерфейс

### Панель подключения

- **Data Path** - путь к папке с базами данных
- **Open Database** - открыть хранилище
- **Database** - выбор активной базы данных
- **Create Database** - создание новой базы данных
- **Drop Database** - удаление выбранной базы данных
- **Refresh** - обновить список баз данных

### Редактор SQL

- Подсветка синтаксиса (ключевые слова, строки, числа)
- Примеры запросов по кнопкам

### Результаты

- **SELECT** - отображается в виде таблицы
- **INSERT / UPDATE / DELETE** - показывается количество затронутых строк

### Горячие клавиши
- F5 - выполнить запрос
- Ctrl+Enter - выполнить запрос
- Ctrl+D - очистить редактор
