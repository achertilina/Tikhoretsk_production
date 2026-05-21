import ctypes
import json
import os

class DatabaseWrapper:
    def __init__(self, lib_path=None):
        if lib_path is None:
            lib_path = self._find_library()
        
        self.lib = ctypes.CDLL(lib_path)
        self._setup_functions()
        self.db_ptr = self.lib.db_create()
    
    def _find_library(self):
        possible_paths = [
            "./build/db_python.so",
            "../build/db_python.so",
            os.path.expanduser("~/keyschemp/Tikhoretsk_production/build/db_python.so")
        ]
        
        for path in possible_paths:
            if os.path.exists(path):
                return path
        
        raise FileNotFoundError(f"Library not found. Checked: {possible_paths}")
    
    def _setup_functions(self):
        self.lib.db_create.restype = ctypes.c_void_p
        self.lib.db_open.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self.lib.db_open.restype = ctypes.c_int
        # db_execute возвращает const char* (указатель на статический буфер)
        self.lib.db_execute.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self.lib.db_execute.restype = ctypes.c_char_p
        self.lib.db_use_database.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self.lib.db_use_database.restype = ctypes.c_int
        self.lib.db_list_databases.argtypes = [ctypes.c_void_p]
        self.lib.db_list_databases.restype = ctypes.c_char_p
        self.lib.db_close.argtypes = [ctypes.c_void_p]
    
    def open(self, path):
        result = self.lib.db_open(self.db_ptr, path.encode('utf-8'))
        return result == 1
    
    def execute(self, sql):
        result_ptr = self.lib.db_execute(self.db_ptr, sql.encode('utf-8'))
        # Копируем строку сразу, пока буфер не перезаписан
        result_str = ctypes.string_at(result_ptr).decode('utf-8')
        return json.loads(result_str)
    
    def use_database(self, db_name):
        result = self.lib.db_use_database(self.db_ptr, db_name.encode('utf-8'))
        return result == 1
    
    def list_databases(self):
        result_ptr = self.lib.db_list_databases(self.db_ptr)
        result_str = ctypes.string_at(result_ptr).decode('utf-8')
        return json.loads(result_str)
    
    def close(self):
        if self.db_ptr:
            self.lib.db_close(self.db_ptr)
            self.db_ptr = None
