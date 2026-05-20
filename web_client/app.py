#!/usr/bin/env python3
from flask import Flask, render_template, request, jsonify, session
import subprocess
import os
import re

app = Flask(__name__)
app.secret_key = 'tikhoretsk_secret_key_2026'

# Путь к скомпилированной C++ программе
DB_CONSOLE = os.path.join(os.path.dirname(__file__), '..', 'build', 'db_console')

def execute_sql(sql: str, database: str = None):
    """Отправляет SQL запрос в C++ СУБД и возвращает результат"""
    
    if not os.path.exists(DB_CONSOLE):
        return {
            'success': False,
            'error': f'СУБД не найдена. Запустите сначала ./scripts/build.sh'
        }
    
    try:
        process = subprocess.Popen(
            [DB_CONSOLE],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        sql_input = sql.strip()
        if not sql_input.endswith(';'):
            sql_input += ';'
        
        sql_input += '\nexit\n'
        
        stdout, stderr = process.communicate(input=sql_input, timeout=5)
        output = stdout
        
        print(f"[DEBUG] SQL: {sql}")
        print(f"[DEBUG] Output: {output[:500]}")
        
        # === SELECT ===
        if 'row(s) returned' in output.lower():
            rows = []
            for line in output.split('\n'):
                line = line.strip()
                # Убираем db> в начале
                if line.startswith('db>'):
                    line = line[3:].strip()
                # Ищем строки с цифрой в начале
                if line and line[0].isdigit():
                    parts = [p for p in line.split(' ') if p]
                    if len(parts) >= 3:
                        rows.append([parts[0], parts[1], parts[2]])
            
            if rows:
                return {
                    'success': True,
                    'is_select': True,
                    'columns': ['id', 'name', 'age'],
                    'rows': rows
                }
        
        # === INSERT/UPDATE/DELETE ===
        match = re.search(r'(\d+)\s*row\(s\)\s*affected', output, re.IGNORECASE)
        if match:
            return {
                'success': True,
                'is_select': False,
                'affected_rows': int(match.group(1))
            }
        
        # === CREATE TABLE ===
        if 'CREATE TABLE' in sql.upper():
            if 'error' not in output.lower():
                return {
                    'success': True,
                    'is_select': False,
                    'message': '✅ Таблица создана'
                }
        
        # === DROP TABLE ===
        if 'DROP TABLE' in sql.upper():
            if 'error' not in output.lower():
                return {
                    'success': True,
                    'is_select': False,
                    'message': '✅ Таблица удалена'
                }
        
        # === CREATE DATABASE ===
        if 'CREATE DATABASE' in sql.upper():
            if 'error' not in output.lower():
                return {
                    'success': True,
                    'is_select': False,
                    'message': '✅ База данных создана'
                }
        
        # === Ошибки ===
        if 'error' in output.lower():
            return {
                'success': False,
                'error': output.strip()
            }
        
        # === Успех без данных ===
        return {
            'success': True,
            'is_select': False,
            'message': '✅ Запрос выполнен успешно'
        }
        
    except subprocess.TimeoutExpired:
        process.kill()
        return {
            'success': False,
            'error': 'Превышено время ожидания (5 секунд)'
        }
    except Exception as e:
        return {
            'success': False,
            'error': str(e)
        }

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/query', methods=['POST'])
def handle_query():
    data = request.get_json()
    sql = data.get('sql', '').strip()
    
    if not sql:
        return jsonify({'success': False, 'error': 'Введите SQL запрос'})
    
    result = execute_sql(sql)
    return jsonify(result)

@app.route('/api/health', methods=['GET'])
def health():
    if os.path.exists(DB_CONSOLE):
        return jsonify({'status': 'ok', 'db_available': True})
    return jsonify({'status': 'ok', 'db_available': False})

if __name__ == '__main__':
    print(f"🏭 Tikhoretsk Production DB - Web Client")
    print(f"📡 Сервер запущен на http://localhost:5000")
    app.run(debug=True, host='0.0.0.0', port=5000)