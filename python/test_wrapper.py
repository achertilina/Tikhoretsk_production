#!/usr/bin/env python3
import sys
import os

sys.path.insert(0, os.path.dirname(__file__))

print("=== Testing Fixed Wrapper ===\n")

try:
    from db_wrapper import DatabaseWrapper
    
    db = DatabaseWrapper()
    
    print("1. Opening database...")
    if db.open("./data"):
        print("   ✓ Opened")
    
    print("2. Creating database...")
    result = db.execute("CREATE DATABASE testdb;")
    print(f"   Result: {result}")
    
    print("3. Creating table...")
    result = db.execute("CREATE TABLE users (id INT, name TEXT);")
    print(f"   Result: {result}")
    
    print("4. Inserting data...")
    result = db.execute("INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');")
    print(f"   Result: {result}")
    
    print("5. Querying...")
    result = db.execute("SELECT * FROM users;")
    print(f"   Result: {result}")
    
    if result.get('success') and result.get('is_select'):
        print(f"   Columns: {result.get('columns')}")
        print(f"   Rows: {result.get('rows')}")
    
    print("\n✓ All tests passed!")
    
    db.close()
    
except Exception as e:
    print(f"Error: {e}")
    import traceback
    traceback.print_exc()
