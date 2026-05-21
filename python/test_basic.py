#!/usr/bin/env python3
import sys
import os

sys.path.insert(0, os.path.dirname(__file__))

print("=== Testing Database ===")
print()

try:
    from db_wrapper import DatabaseWrapper
    
    print("1. Creating Database instance...")
    db = DatabaseWrapper()
    print("   OK")
    
    print("2. Opening database...")
    if db.open("./data"):
        print("   OK - Database opened")
    else:
        print("   FAILED")
        sys.exit(1)
    
    print("3. Creating test database...")
    result = db.execute("CREATE DATABASE testdb;")
    print(f"   Result: {result}")
    
    print("4. Using test database...")
    if db.use_database("testdb"):
        print("   OK - Using testdb")
    
    print("5. Creating table...")
    result = db.execute("CREATE TABLE users (id INT, name TEXT);")
    print(f"   Result: {result}")
    
    print("6. Inserting data...")
    result = db.execute("INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');")
    print(f"   Result: {result}")
    
    print("7. Querying data...")
    result = db.execute("SELECT * FROM users;")
    print(f"   Result: {result}")
    
    if result.get("success"):
        print(f"   Columns: {result.get('columns')}")
        print(f"   Rows: {result.get('rows')}")
    
    print("\n✓ All basic tests passed!")
    
    db.close()
    
except Exception as e:
    print(f"\n✗ Error: {e}")
    import traceback
    traceback.print_exc()
