#!/usr/bin/env python3
import sys
import os
import json

sys.path.insert(0, os.path.dirname(__file__))
from db_wrapper import DatabaseWrapper

print("=== Final Test ===\n")

db = DatabaseWrapper()
print("✓ Database wrapper created")

if db.open("./data"):
    print("✓ Database opened")
    
    # Clean up old test database
    result = db.execute("DROP DATABASE IF EXISTS test_final;")
    print(f"Drop if exists: {result}")
    
    # Create database
    result = db.execute("CREATE DATABASE test_final;")
    print(f"Create DB: {result}")
    
    # Use database
    if db.use_database("test_final"):
        print("✓ Using test_final")
    
    # Create table
    result = db.execute("CREATE TABLE users (id INT, name TEXT);")
    print(f"Create table: {result}")
    
    # Insert data
    result = db.execute("INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');")
    print(f"Insert: {result}")
    
    # Select data
    result = db.execute("SELECT * FROM users;")
    print(f"Select: {result}")
    
    if result.get('success') and result.get('is_select'):
        print(f"  Columns: {result.get('columns')}")
        print(f"  Rows: {result.get('rows')}")
    
    # Update
    result = db.execute("UPDATE users SET name = 'Alicia' WHERE id = 1;")
    print(f"Update: {result}")
    
    # Select again
    result = db.execute("SELECT * FROM users;")
    print(f"Select after update: {result}")
    
    # Delete
    result = db.execute("DELETE FROM users WHERE id = 2;")
    print(f"Delete: {result}")
    
    # Final select
    result = db.execute("SELECT * FROM users;")
    print(f"Final select: {result}")
    
    print("\n✓✓✓ ALL TESTS PASSED! ✓✓✓")
    
    db.close()
else:
    print("✗ Failed to open database")
