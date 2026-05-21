#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext, simpledialog
import threading
import os
import sys
import re

sys.path.insert(0, os.path.dirname(__file__))
from db_wrapper import DatabaseWrapper

class RoundedFrame(tk.Canvas):
    """Кастомная рамка со скругленными углами"""
    def __init__(self, parent, corner_radius=15, bg_color='#333333', **kwargs):
        self.corner_radius = corner_radius
        self.bg_color = bg_color
        super().__init__(parent, highlightthickness=0, bg=parent.cget('bg'), **kwargs)
        self.configure(bg=parent.cget('bg'))
        self.create_rounded_rect()
    
    def create_rounded_rect(self):
        width = self.winfo_reqwidth()
        height = self.winfo_reqheight()
        r = self.corner_radius
        
        self.rounded_rect = self.create_polygon(
            r, 0, width-r, 0,
            width, 0, width, r,
            width, height-r, width, height,
            width-r, height, r, height,
            0, height, 0, height-r,
            0, r, 0, 0,
            fill=self.bg_color,
            outline=self.bg_color,
            smooth=True
        )
        
        self.bind('<Configure>', self.on_resize)
    
    def on_resize(self, event):
        self.delete(self.rounded_rect)
        width = event.width
        height = event.height
        r = self.corner_radius
        
        self.rounded_rect = self.create_polygon(
            r, 0, width-r, 0,
            width, 0, width, r,
            width, height-r, width, height,
            width-r, height, r, height,
            0, height, 0, height-r,
            0, r, 0, 0,
            fill=self.bg_color,
            outline=self.bg_color,
            smooth=True
        )

class SQLSyntaxHighlighting:
    """Класс для подсветки синтаксиса SQL"""
    keywords = [
        'SELECT', 'FROM', 'WHERE', 'INSERT', 'UPDATE', 'DELETE', 'CREATE', 'DROP',
        'TABLE', 'DATABASE', 'INT', 'TEXT', 'VALUES', 'SET', 'AND', 'OR', 'NOT',
        'INTO', 'ALTER', 'INDEX', 'VIEW', 'PRIMARY', 'FOREIGN', 'KEY', 'REFERENCES'
    ]
    
    def __init__(self, text_widget):
        self.text = text_widget
        self.setup_tags()
        self.bind_events()
    
    def setup_tags(self):
        self.text.tag_configure('keyword', foreground='#0000FF', font=('Courier', 12, 'bold'))
        self.text.tag_configure('string', foreground='#008000')
        self.text.tag_configure('number', foreground='#FF0000')
        self.text.tag_configure('comment', foreground='#808080')
        self.text.tag_configure('operator', foreground='#FF00FF')
    
    def bind_events(self):
        self.text.bind('<KeyRelease>', lambda e: self.highlight())
        self.text.bind('<FocusIn>', lambda e: self.highlight())
    
    def highlight(self):
        for tag in ['keyword', 'string', 'number', 'comment', 'operator']:
            self.text.tag_remove(tag, '1.0', tk.END)
        
        text = self.text.get('1.0', tk.END)
        
        for match in re.finditer(r"'[^']*'", text):
            start = f"1.0 + {match.start()} chars"
            end = f"1.0 + {match.end()} chars"
            self.text.tag_add('string', start, end)
        
        for match in re.finditer(r'\b\d+\b', text):
            start = f"1.0 + {match.start()} chars"
            end = f"1.0 + {match.end()} chars"
            self.text.tag_add('number', start, end)
        
        text_upper = text.upper()
        for keyword in self.keywords:
            for match in re.finditer(rf'\b{keyword}\b', text_upper):
                start = f"1.0 + {match.start()} chars"
                end = f"1.0 + {match.end()} chars"
                self.text.tag_add('keyword', start, end)
        
        for match in re.finditer(r'[=<>!+*/-]', text):
            if match.start() > 0:
                if not re.match(r'\s', text[match.start()-1:match.start()]):
                    start = f"1.0 + {match.start()} chars"
                    end = f"1.0 + {match.end()} chars"
                    self.text.tag_add('operator', start, end)

class ModernDatabaseGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Tikhoretsk Production Database")
        self.root.geometry("1400x850")
        
        self.center_window()
        self.setup_theme()
        
        self.db = None
        self.current_db = None
        
        self.setup_ui()
        
        if os.path.exists("./data"):
            self.open_db()
    
    def center_window(self):
        self.root.update_idletasks()
        width = 1400
        height = 850
        x = (self.root.winfo_screenwidth() // 2) - (width // 2)
        y = (self.root.winfo_screenheight() // 2) - (height // 2)
        self.root.geometry(f'{width}x{height}+{x}+{y}')
    
    def setup_theme(self):
        style = ttk.Style()
        
        self.colors = {
            'bg': '#1e1e1e',
            'fg': '#ffffff',
            'select_bg': '#404040',
            'select_fg': '#ffffff',
            'button_bg': '#3c3c3c',
            'button_fg': '#ffffff',
            'frame_bg': '#2d2d2d',
            'input_bg': '#3c3c3c',
            'input_fg': '#ffffff',
            'success': '#4caf50',
            'error': '#f44336',
            'info': '#2196f3',
            'warning': '#ff9800',
            'combo_fg': '#000000',
            'title_bg': '#4caf50',
            'results_bg': '#e8f5e9'  # Светло-салатовый для области результатов
        }
        
        style.theme_use('clam')
        
        style.configure('TFrame', background=self.colors['bg'])
        style.configure('TLabel', background=self.colors['bg'], foreground=self.colors['fg'], 
                       font=('Segoe UI', 10))
        
        style.configure('TButton', background=self.colors['button_bg'], foreground=self.colors['button_fg'], 
                       borderwidth=0, focusthickness=3, focuscolor='none', padding=8,
                       font=('Segoe UI', 10))
        style.map('TButton',
                 background=[('active', '#4caf50'), ('pressed', '#45a049')],
                 foreground=[('active', 'white'), ('pressed', 'white')])
        
        style.configure('Success.TButton', background=self.colors['success'], font=('Segoe UI', 10))
        style.configure('Info.TButton', background=self.colors['info'], font=('Segoe UI', 10))
        style.configure('Warning.TButton', background=self.colors['warning'], font=('Segoe UI', 10))
        
        style.configure('TEntry', fieldbackground=self.colors['input_bg'], 
                       foreground=self.colors['input_fg'],
                       font=('Segoe UI', 11))
        
        style.configure('TCombobox', 
                       fieldbackground='#ffffff',
                       foreground='#000000',
                       font=('Segoe UI', 11),
                       selectbackground='#4caf50',
                       selectforeground='#ffffff')
        style.map('TCombobox',
                 fieldbackground=[('readonly', '#ffffff')],
                 foreground=[('readonly', '#000000')])
        
        style.configure('Treeview', 
                       background=self.colors['results_bg'],
                       foreground='#000000',
                       fieldbackground=self.colors['results_bg'],
                       rowheight=28,
                       font=('Segoe UI', 10))
        style.map('Treeview',
                 background=[('selected', '#4caf50')],
                 foreground=[('selected', '#ffffff')])
        
        style.configure('Treeview.Heading', 
                       background=self.colors['button_bg'], 
                       foreground='#ffffff',
                       font=('Segoe UI', 11, 'bold'))
        style.map('Treeview.Heading',
                 background=[('active', self.colors['select_bg'])])
        
        self.root.configure(bg=self.colors['bg'])
    
    def setup_ui(self):
        main_container = tk.Frame(self.root, bg=self.colors['bg'])
        main_container.pack(expand=True, fill=tk.BOTH)
        
        center_frame = RoundedFrame(main_container, corner_radius=20, bg_color=self.colors['frame_bg'])
        center_frame.pack(expand=True, fill=tk.BOTH, padx=40, pady=40)
        
        content_frame = tk.Frame(center_frame, bg=self.colors['frame_bg'])
        content_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
        
        # Заголовок
        title_container = tk.Frame(content_frame, bg=self.colors['frame_bg'])
        title_container.pack(fill=tk.X, pady=(0, 20))
        
        title_rounded = RoundedFrame(title_container, corner_radius=15, bg_color=self.colors['title_bg'])
        title_rounded.pack(expand=True, pady=5)
        
        inner_frame = tk.Frame(title_rounded, bg=self.colors['title_bg'], padx=40, pady=15)
        inner_frame.pack(padx=2, pady=2)
        
        title_label = tk.Label(inner_frame, 
                              text="Tikhoretsk Production Database", 
                              font=('Segoe UI', 22, 'bold'),
                              fg='white',
                              bg=self.colors['title_bg'])
        title_label.pack()
        
        subtitle_label = tk.Label(inner_frame,
                                 text="Database Management System",
                                 font=('Segoe UI', 11),
                                 fg='#e0e0e0',
                                 bg=self.colors['title_bg'])
        subtitle_label.pack(pady=(5, 0))
        
        # Панель подключения
        conn_frame = tk.LabelFrame(content_frame, text=" Connection ", 
                                   font=('Segoe UI', 11, 'bold'),
                                   fg=self.colors['fg'],
                                   bg=self.colors['frame_bg'],
                                   bd=1, relief=tk.RIDGE)
        conn_frame.pack(fill=tk.X, pady=(0, 15), padx=5)
        
        path_frame = tk.Frame(conn_frame, bg=self.colors['frame_bg'])
        path_frame.pack(fill=tk.X, pady=8, padx=10)
        
        tk.Label(path_frame, text="Data Path:", font=('Segoe UI', 11),
                bg=self.colors['frame_bg'], fg=self.colors['fg']).pack(side=tk.LEFT, padx=5)
        self.path_var = tk.StringVar(value="./data")
        path_entry = ttk.Entry(path_frame, textvariable=self.path_var, width=50)
        path_entry.pack(side=tk.LEFT, padx=5)
        
        open_btn = ttk.Button(path_frame, text="Open Database", command=self.open_db, style='Info.TButton')
        open_btn.pack(side=tk.LEFT, padx=5)
        
        db_frame = tk.Frame(conn_frame, bg=self.colors['frame_bg'])
        db_frame.pack(fill=tk.X, pady=8, padx=10)
        
        tk.Label(db_frame, text="Database:", font=('Segoe UI', 11),
                bg=self.colors['frame_bg'], fg=self.colors['fg']).pack(side=tk.LEFT, padx=5)
        self.db_var = tk.StringVar()
        self.db_combo = ttk.Combobox(db_frame, textvariable=self.db_var, state="readonly", width=30)
        self.db_combo.pack(side=tk.LEFT, padx=5)
        self.db_combo.bind('<<ComboboxSelected>>', self.on_db_select)
        
        create_btn = ttk.Button(db_frame, text="Create Database", command=self.create_db, style='Success.TButton')
        create_btn.pack(side=tk.LEFT, padx=2)
        
        drop_btn = ttk.Button(db_frame, text="Drop Database", command=self.drop_db, style='Warning.TButton')
        drop_btn.pack(side=tk.LEFT, padx=2)
        
        refresh_btn = ttk.Button(db_frame, text="Refresh", command=self.refresh_databases)
        refresh_btn.pack(side=tk.LEFT, padx=2)
        
        # Редактор запросов
        query_frame = tk.LabelFrame(content_frame, text=" SQL Query Editor ", 
                                    font=('Segoe UI', 11, 'bold'),
                                    fg=self.colors['fg'],
                                    bg=self.colors['frame_bg'],
                                    bd=1, relief=tk.RIDGE)
        query_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 15), padx=5)
        
        self.query_text = scrolledtext.ScrolledText(query_frame, height=12, 
                                                    font=('Courier New', 12),
                                                    bg=self.colors['input_bg'],
                                                    fg=self.colors['input_fg'],
                                                    insertbackground='white',
                                                    selectbackground='#4caf50',
                                                    bd=0)
        self.query_text.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        self.syntax = SQLSyntaxHighlighting(self.query_text)
        
        btn_frame = tk.Frame(query_frame, bg=self.colors['frame_bg'])
        btn_frame.pack(fill=tk.X, pady=8, padx=10)
        
        execute_btn = ttk.Button(btn_frame, text="Execute (F5)", command=self.execute_query, 
                                 style='Success.TButton')
        execute_btn.pack(side=tk.LEFT, padx=2)
        
        clear_btn = ttk.Button(btn_frame, text="Clear", command=self.clear_query)
        clear_btn.pack(side=tk.LEFT, padx=2)
        
        example_frame = tk.Frame(btn_frame, bg=self.colors['frame_bg'])
        example_frame.pack(side=tk.LEFT, padx=20)
        
        tk.Label(example_frame, text="Examples:", font=('Segoe UI', 10),
                bg=self.colors['frame_bg'], fg=self.colors['fg']).pack(side=tk.LEFT)
        
        example_select_btn = ttk.Button(example_frame, text="SELECT * FROM users", 
                                       command=lambda: self.insert_example("SELECT * FROM users;"))
        example_select_btn.pack(side=tk.LEFT, padx=2)
        
        example_insert_btn = ttk.Button(example_frame, text="INSERT INTO users", 
                                       command=lambda: self.insert_example("INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');"))
        example_insert_btn.pack(side=tk.LEFT, padx=2)
        
        # Область результатов с салатовым фоном
        results_frame = tk.LabelFrame(content_frame, text=" Query Results ", 
                                      font=('Segoe UI', 11, 'bold'),
                                      fg=self.colors['fg'],
                                      bg=self.colors['frame_bg'],
                                      bd=1, relief=tk.RIDGE)
        results_frame.pack(fill=tk.BOTH, expand=True, padx=5)
        
        tree_container = tk.Frame(results_frame, bg=self.colors['results_bg'])
        tree_container.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Создаем Treeview с салатовым фоном
        self.tree = ttk.Treeview(tree_container, show="headings", height=15)
        vsb = ttk.Scrollbar(tree_container, orient="vertical", command=self.tree.yview)
        hsb = ttk.Scrollbar(tree_container, orient="horizontal", command=self.tree.xview)
        self.tree.configure(yscrollcommand=vsb.set, xscrollcommand=hsb.set)
        
        self.tree.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        vsb.grid(row=0, column=1, sticky=(tk.N, tk.S))
        hsb.grid(row=1, column=0, sticky=(tk.W, tk.E))
        
        tree_container.columnconfigure(0, weight=1)
        tree_container.rowconfigure(0, weight=1)
        
        # Настройка цветов строк с салатовым фоном
        self.tree.tag_configure('odd', background='#c8e6c9', foreground='#000000', font=('Segoe UI', 10))
        self.tree.tag_configure('even', background='#e8f5e9', foreground='#000000', font=('Segoe UI', 10))
        
        # Статус бар
        status_frame = tk.Frame(self.root, bg=self.colors['bg'])
        status_frame.pack(side=tk.BOTTOM, fill=tk.X)
        
        self.status_var = tk.StringVar(value="Ready")
        status_label = tk.Label(status_frame, textvariable=self.status_var, 
                                relief=tk.SUNKEN, anchor=tk.W, padx=5, pady=3,
                                font=('Segoe UI', 10), bg=self.colors['bg'], fg=self.colors['fg'])
        status_label.pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        self.progress = ttk.Progressbar(status_frame, mode='indeterminate', length=100)
        
        self.root.bind('<F5>', lambda e: self.execute_query())
        self.root.bind('<Control-Return>', lambda e: self.execute_query())
        self.root.bind('<Control-d>', lambda e: self.clear_query())
    
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
                self.status_var.set("Failed to connect")
        except Exception as e:
            messagebox.showerror("Error", str(e))
            self.status_var.set(f"Error: {str(e)[:50]}")
    
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
                self.status_var.set(f"Using database: {dbs[0]}")
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
                self.status_var.set(f"Failed to switch to {db_name}")
    
    def create_db(self):
        db_name = simpledialog.askstring("Create Database", "Enter database name:", parent=self.root)
        if db_name and self.db:
            result = self.db.execute(f"CREATE DATABASE {db_name};")
            if result.get("success"):
                self.status_var.set(f"Database '{db_name}' created successfully!")
                self.refresh_databases()
            else:
                messagebox.showerror("Error", result.get("error", "Unknown error"))
                self.status_var.set(f"Failed to create database")
    
    def drop_db(self):
        if not self.current_db:
            messagebox.showwarning("Warning", "No database selected")
            return
        
        if messagebox.askyesno("Confirm Delete", f"Delete database '{self.current_db}'?\nThis action cannot be undone!", 
                               parent=self.root):
            result = self.db.execute(f"DROP DATABASE {self.current_db};")
            if result.get("success"):
                self.status_var.set(f"Database '{self.current_db}' dropped")
                self.refresh_databases()
            else:
                messagebox.showerror("Error", result.get("error", "Unknown error"))
                self.status_var.set(f"Failed to drop database")
    
    def execute_query(self):
        if not self.db:
            messagebox.showwarning("Warning", "Please open a database first")
            return
        
        sql = self.query_text.get("1.0", tk.END).strip()
        if not sql:
            messagebox.showinfo("Info", "Please enter a SQL query")
            return
        
        self.progress.pack(side=tk.RIGHT, padx=5)
        self.progress.start(10)
        self.status_var.set("Executing query...")
        
        def run():
            try:
                result = self.db.execute(sql)
                self.root.after(0, lambda: self.display_result(result))
                self.root.after(0, self.progress.stop)
                self.root.after(0, lambda: self.progress.pack_forget())
            except Exception as e:
                self.root.after(0, lambda: messagebox.showerror("Error", str(e)))
                self.root.after(0, lambda: self.status_var.set(f"Error: {str(e)[:50]}"))
                self.root.after(0, self.progress.stop)
                self.root.after(0, lambda: self.progress.pack_forget())
        
        threading.Thread(target=run, daemon=True).start()
    
    def display_result(self, result):
        for item in self.tree.get_children():
            self.tree.delete(item)
        
        if result.get("success"):
            if result.get("is_select"):
                columns = result.get("columns", [])
                rows = result.get("rows", [])
                
                self.tree["columns"] = columns
                for col in columns:
                    self.tree.heading(col, text=col.upper(), anchor='center')
                    self.tree.column(col, width=150, anchor='center')
                
                for i, row in enumerate(rows):
                    tag = 'even' if i % 2 == 0 else 'odd'
                    self.tree.insert("", "end", values=row, tags=(tag,))
                
                self.status_var.set(f"Query returned {len(rows)} row(s)")
            else:
                affected = result.get("affected_rows", 0)
                self.status_var.set(f"Query executed successfully. {affected} row(s) affected")
        else:
            error = result.get("error", "Unknown error")
            messagebox.showerror("Query Error", error)
            self.status_var.set(f"Query error: {error[:50]}")
    
    def clear_query(self):
        self.query_text.delete("1.0", tk.END)
        self.status_var.set("Query cleared")
    
    def insert_example(self, sql):
        self.query_text.delete("1.0", tk.END)
        self.query_text.insert("1.0", sql)
        self.syntax.highlight()

def main():
    root = tk.Tk()
    app = ModernDatabaseGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
