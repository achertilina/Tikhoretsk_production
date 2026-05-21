#!/usr/bin/env python3
import ctypes
import json
import os

# Загружаем библиотеку
lib = ctypes.CDLL("../build/db_python.so")

# Настройка функций
lib.db_create.restype = ctypes.c_void_p
lib.db_open.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.db_open.restype = ctypes.c_int
lib.db_execute.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.db_execute.restype = ctypes.c_char_p
lib.db_close.argtypes = [ctypes.c_void_p]
lib.free_string.argtypes = [ctypes.c_char_p]

# Создаем базу данных
db = lib.db_create()
print("Database created")

# Открываем
if lib.db_open(db, b"./data"):
    print("Database opened")
    
    # Выполняем запрос
    sql = b"CREATE DATABASE testdb2;"
    result_ptr = lib.db_execute(db, sql)
    
    # Конвертируем результат
    result_str = ctypes.c_char_p(result_ptr).value
    if result_str:
        result_text = result_str.decode('utf-8')
        print(f"Result: {result_text}")
        
        # Освобождаем память
        lib.free_string(result_ptr)
    
    # Закрываем
    lib.db_close(db)
    print("Done")
else:
    print("Failed to open")
