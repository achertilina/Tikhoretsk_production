# Отчёт о реализации MVP СУБД TikhoretskDB

## Паттерн проектирования

**Название:** Facade (Фасад)

**Где применён:** Класс `Database` в `src/client/database.h`

**Почему выбран:** Скрывает сложность парсинга и выполнения SQL от клиента, давая простой интерфейс.

## Соответствие критериям

- [x] Паттерн применён (Facade)
- [x] Клиент через C++ библиотеку
- [x] build.sh устанавливает зависимости
- [x] run.sh принимает host/port
- [x] CREATE/DROP DATABASE/TABLE работают
- [x] SELECT/INSERT/UPDATE/DELETE работают
- [x] Интерфейс показывает результат

## Использование

```bash
./scripts/build.sh
./scripts/run.sh localhost 5432

