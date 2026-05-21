#!/usr/bin/env python3
import sys
import os
import ctypes

# Загружаем библиотеку напрямую для отладки
lib_path = os.path.expanduser("~/keyschemp/Tikhoretsk_production/build/db_python.so")
print(f"Loading library: {lib_path}")

lib = ctypes.CDLL(lib_path)

# Настраиваем функции
lib.db_create.restype = ctypes.c_void_p
lib.db_open.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.db_open.restype = ctypes.c_int
lib.db_execute.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.db_execute.restype = ctypes.c_char_p
lib.db_close.argtypes = [ctypes.c_void_p]
lib.free_string.argtypes = [ctypes.c_char_p]

print("1. Creating database instance...")
db_ptr = lib.db_create()
print(f"   Pointer: {db_ptr}")

print("2. Opening database...")
result = lib.db_open(db_ptr, b"./data")
print(f"   Result: {result}")

if result:
    print("3. Executing CREATE DATABASE...")
    sql = b"CREATE DATABASE testdb;"
    result_ptr = lib.db_execute(db_ptr, sql)
    
    if result_ptr:
        # Правильный способ получить строку из char*
        result_str = ctypes.c_char_p(result_ptr).value.decode('utf-8')
        print(f"   Result string: {result_str}")
        # Освобождаем память
        lib.free_string(result_ptr)
    else:
        print("   Got NULL pointer")
    
    print("4. Executing SELECT...")
    sql2 = b"SELECT 1;"
    result_ptr2 = lib.db_execute(db_ptr, sql2)
    if result_ptr2:
        result_str2 = ctypes.c_char_p(result_ptr2).value.decode('utf-8')
        print(f"   Result: {result_str2}")
        lib.free_string(result_ptr2)
    
    print("5. Closing database...")
    lib.db_close(db_ptr)
    print("   Done")
else:
    print("Failed to open database")
