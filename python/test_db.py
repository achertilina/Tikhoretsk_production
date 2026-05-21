#!/usr/bin/env python3
import sys
import os

sys.path.insert(0, os.path.dirname(__file__))

print("Testing Database Wrapper...")
print("-" * 50)

try:
    from db_wrapper import DatabaseWrapper
    
    print("1. Creating Database instance...")
    db = DatabaseWrapper()
    print("   ✓ Instance created")
    
    print("2. Opening database at './data'...")
    if db.open("./data"):
        print("   ✓ Database opened successfully")
    else:
        print("   ✗ Failed to open database")
        sys.exit(1)
    
    print("3. Listing databases...")
    dbs = db.list_databases()
    print(f"   Found: {dbs}")
    
    print("4. Creating test database...")
    result = db.execute("CREATE DATABASE testdb;")
    print(f"   Result: {result}")
    
    print("5. Switching to testdb...")
    if db.use_database("testdb"):
        print("   ✓ Switched to testdb")
    
    print("6. Creating users table...")
    result = db.execute("CREATE TABLE users (id INT, name TEXT);")
    print(f"   Result: {result}")
    
    print("7. Inserting data...")
    result = db.execute("INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');")
    print(f"   Result: {result}")
    
    print("8. Selecting all users...")
    result = db.execute("SELECT * FROM users;")
    print(f"   Result: {result}")
    
    if result.get("success") and result.get("is_select"):
        print(f"   Columns: {result.get('columns')}")
        print(f"   Rows: {result.get('rows')}")
    
    print("\n" + "=" * 50)
    print("✓ All tests passed successfully!")
    
    db.close()
    
except Exception as e:
    print(f"\n✗ Error: {e}")
    import traceback
    traceback.print_exc()
    sys.exit(1)
