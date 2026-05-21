#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext, simpledialog
import threading
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from db_wrapper import DatabaseWrapper

class DatabaseGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Tikhoretsk Production DB - Python GUI")
        self.root.geometry("1200x700")
        
        self.db = None
        self.current_db = None
        
        self.setup_ui()
        
        # Auto-open if data directory exists
        if os.path.exists("./data"):
            self.open_db()
    
    def setup_ui(self):
        # Main container
        main_frame = ttk.Frame(self.root, padding="5")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Connection panel
        conn_frame = ttk.LabelFrame(main_frame, text="Connection", padding="10")
        conn_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Path selection
        path_frame = ttk.Frame(conn_frame)
        path_frame.pack(fill=tk.X, pady=2)
        ttk.Label(path_frame, text="Data Path:").pack(side=tk.LEFT, padx=5)
        self.path_var = tk.StringVar(value="./data")
        path_entry = ttk.Entry(path_frame, textvariable=self.path_var, width=50)
        path_entry.pack(side=tk.LEFT, padx=5)
        ttk.Button(path_frame, text="Open", command=self.open_db).pack(side=tk.LEFT, padx=5)
        
        # Database management
        db_frame = ttk.Frame(conn_frame)
        db_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(db_frame, text="Database:").pack(side=tk.LEFT, padx=5)
        self.db_var = tk.StringVar()
        self.db_combo = ttk.Combobox(db_frame, textvariable=self.db_var, state="readonly", width=30)
        self.db_combo.pack(side=tk.LEFT, padx=5)
        self.db_combo.bind('<<ComboboxSelected>>', self.on_db_select)
        
        ttk.Button(db_frame, text="Create Database", command=self.create_db).pack(side=tk.LEFT, padx=2)
        ttk.Button(db_frame, text="Drop Database", command=self.drop_db).pack(side=tk.LEFT, padx=2)
        ttk.Button(db_frame, text="Refresh", command=self.refresh_databases).pack(side=tk.LEFT, padx=2)
        
        # Query editor
        query_frame = ttk.LabelFrame(main_frame, text="SQL Query", padding="10")
        query_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        self.query_text = scrolledtext.ScrolledText(query_frame, height=12, font=("Courier", 10))
        self.query_text.pack(fill=tk.BOTH, expand=True)
        
        # Buttons
        btn_frame = ttk.Frame(query_frame)
        btn_frame.pack(fill=tk.X, pady=5)
        
        ttk.Button(btn_frame, text="Execute (F5)", command=self.execute_query).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_frame, text="Clear", command=self.clear_query).pack(side=tk.LEFT, padx=2)
        
        # Example queries
        example_frame = ttk.Frame(btn_frame)
        example_frame.pack(side=tk.LEFT, padx=20)
        ttk.Label(example_frame, text="Examples:").pack(side=tk.LEFT)
        ttk.Button(example_frame, text="SELECT * FROM users", 
                   command=lambda: self.insert_example("SELECT * FROM users;")).pack(side=tk.LEFT, padx=2)
        ttk.Button(example_frame, text="INSERT", 
                   command=lambda: self.insert_example("INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');")).pack(side=tk.LEFT, padx=2)
        
        # Results area
        results_frame = ttk.LabelFrame(main_frame, text="Results", padding="10")
        results_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        # Treeview for results
        tree_container = ttk.Frame(results_frame)
        tree_container.pack(fill=tk.BOTH, expand=True)
        
        self.tree = ttk.Treeview(tree_container, show="headings")
        vsb = ttk.Scrollbar(tree_container, orient="vertical", command=self.tree.yview)
        hsb = ttk.Scrollbar(tree_container, orient="horizontal", command=self.tree.xview)
        self.tree.configure(yscrollcommand=vsb.set, xscrollcommand=hsb.set)
        
        self.tree.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        vsb.grid(row=0, column=1, sticky=(tk.N, tk.S))
        hsb.grid(row=1, column=0, sticky=(tk.W, tk.E))
        
        tree_container.columnconfigure(0, weight=1)
        tree_container.rowconfigure(0, weight=1)
        
        # Status bar
        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status_bar.pack(side=tk.BOTTOM, fill=tk.X)
        
        # Bind keyboard shortcuts
        self.root.bind('<F5>', lambda e: self.execute_query())
        self.root.bind('<Control-Return>', lambda e: self.execute_query())
    
    def open_db(self):
        path = self.path_var.get()
        try:
            if self.db:
                self.db.close()
            
            self.db = DatabaseWrapper()
            if self.db.open(path):
                self.status_var.set(f"Connected to {path}")
                self.refresh_databases()
            else:
                messagebox.showerror("Error", "Failed to open database")
        except Exception as e:
            messagebox.showerror("Error", str(e))
    
    def refresh_databases(self):
        if not self.db:
            return
        try:
            dbs = self.db.list_databases()
            self.db_combo['values'] = dbs
            if dbs:
                self.db_combo.set(dbs[0])
                self.current_db = dbs[0]
                self.db.use_database(dbs[0])
            else:
                self.db_combo.set('')
                self.current_db = None
        except Exception as e:
            self.status_var.set(f"Error refreshing: {e}")
    
    def on_db_select(self, event=None):
        db_name = self.db_var.get()
        if db_name and self.db:
            if self.db.use_database(db_name):
                self.current_db = db_name
                self.status_var.set(f"Using database: {db_name}")
            else:
                messagebox.showerror("Error", f"Failed to switch to {db_name}")
    
    def create_db(self):
        db_name = simpledialog.askstring("Create Database", "Enter database name:")
        if db_name and self.db:
            result = self.db.execute(f"CREATE DATABASE {db_name};")
            if result.get("success"):
                self.status_var.set(f"Database '{db_name}' created")
                self.refresh_databases()
            else:
                messagebox.showerror("Error", result.get("error", "Unknown error"))
    
    def drop_db(self):
        if not self.current_db:
            messagebox.showwarning("Warning", "No database selected")
            return
        
        if messagebox.askyesno("Confirm", f"Delete database '{self.current_db}'?\nThis cannot be undone!"):
            result = self.db.execute(f"DROP DATABASE {self.current_db};")
            if result.get("success"):
                self.status_var.set(f"Database '{self.current_db}' dropped")
                self.refresh_databases()
            else:
                messagebox.showerror("Error", result.get("error", "Unknown error"))
    
    def execute_query(self):
        if not self.db:
            messagebox.showwarning("Warning", "Please open a database first")
            return
        
        sql = self.query_text.get("1.0", tk.END).strip()
        if not sql:
            return
        
        def run():
            try:
                result = self.db.execute(sql)
                self.root.after(0, lambda: self.display_result(result))
            except Exception as e:
                self.root.after(0, lambda: messagebox.showerror("Error", str(e)))
                self.root.after(0, lambda: self.status_var.set("Error"))
        
        self.status_var.set("Executing query...")
        threading.Thread(target=run, daemon=True).start()
    
    def display_result(self, result):
        # Clear tree
        for item in self.tree.get_children():
            self.tree.delete(item)
        
        if result.get("success"):
            if result.get("is_select"):
                columns = result.get("columns", [])
                rows = result.get("rows", [])
                
                self.tree["columns"] = columns
                for col in columns:
                    self.tree.heading(col, text=col)
                    self.tree.column(col, width=100)
                
                for row in rows:
                    self.tree.insert("", "end", values=row)
                
                self.status_var.set(f"Query returned {len(rows)} row(s)")
            else:
                affected = result.get("affected_rows", 0)
                self.status_var.set(f"Query executed. {affected} row(s) affected")
        else:
            error = result.get("error", "Unknown error")
            messagebox.showerror("Query Error", error)
            self.status_var.set(f"Error: {error}")
    
    def clear_query(self):
        self.query_text.delete("1.0", tk.END)
    
    def insert_example(self, sql):
        self.query_text.delete("1.0", tk.END)
        self.query_text.insert("1.0", sql)

def main():
    root = tk.Tk()
    app = DatabaseGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
