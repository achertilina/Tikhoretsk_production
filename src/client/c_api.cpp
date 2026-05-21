#include "client/database.h"
#include "client/result.h"
#include <cstring>
#include <string>
#include <cstdlib>

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

// Глобальный статический буфер для результатов (не потокобезопасно, но для теста пойдет)
static char g_result_buffer[65536];

extern "C" {
    
    EXPORT void* db_create() {
        return new Database();
    }
    
    EXPORT int db_open(void* db, const char* path) {
        if (!db || !path) return 0;
        Database* database = static_cast<Database*>(db);
        return database->open(path) ? 1 : 0;
    }
    
    EXPORT const char* db_execute(void* db, const char* sql) {
        if (!db || !sql) {
            std::strcpy(g_result_buffer, "{\"success\":false,\"error\":\"Invalid parameters\"}");
            return g_result_buffer;
        }
        
        Database* database = static_cast<Database*>(db);
        
        try {
            ClientResult result = database->execute(sql);
            std::string json;
            
            if (!result.success()) {
                json = "{\"success\":false,\"error\":\"" + result.errorMessage() + "\"}";
            } else if (result.isSelect()) {
                json = "{\"success\":true,\"is_select\":true,\"columns\":[";
                
                const auto& cols = result.columnNames();
                for (size_t i = 0; i < cols.size(); ++i) {
                    if (i > 0) json += ",";
                    json += "\"" + cols[i] + "\"";
                }
                json += "],\"rows\":[";
                
                for (size_t row = 0; row < result.rowCount(); ++row) {
                    if (row > 0) json += ",";
                    json += "[";
                    auto row_data = result.getRowAsStrings(row);
                    for (size_t col = 0; col < row_data.size(); ++col) {
                        if (col > 0) json += ",";
                        std::string escaped = row_data[col];
                        size_t pos = 0;
                        while ((pos = escaped.find('"', pos)) != std::string::npos) {
                            escaped.replace(pos, 1, "\\\"");
                            pos += 2;
                        }
                        json += "\"" + escaped + "\"";
                    }
                    json += "]";
                }
                json += "]}";
            } else {
                json = "{\"success\":true,\"is_select\":false,\"affected_rows\":" + 
                        std::to_string(result.affectedRows()) + "}";
            }
            
            std::strcpy(g_result_buffer, json.c_str());
            return g_result_buffer;
            
        } catch (const std::exception& e) {
            std::string msg = std::string("{\"success\":false,\"error\":\"") + e.what() + "\"}";
            std::strcpy(g_result_buffer, msg.c_str());
            return g_result_buffer;
        }
    }
    
    EXPORT int db_use_database(void* db, const char* db_name) {
        if (!db || !db_name) return 0;
        Database* database = static_cast<Database*>(db);
        return database->useDatabase(db_name) ? 1 : 0;
    }
    
    EXPORT const char* db_list_databases(void* db) {
        if (!db) {
            std::strcpy(g_result_buffer, "[]");
            return g_result_buffer;
        }
        
        Database* database = static_cast<Database*>(db);
        std::vector<std::string> dbs = database->listDatabases();
        
        std::string json = "[";
        for (size_t i = 0; i < dbs.size(); ++i) {
            if (i > 0) json += ",";
            json += "\"" + dbs[i] + "\"";
        }
        json += "]";
        
        std::strcpy(g_result_buffer, json.c_str());
        return g_result_buffer;
    }
    
    EXPORT void db_close(void* db) {
        if (db) {
            Database* database = static_cast<Database*>(db);
            database->close();
            delete database;
        }
    }
    
    // Больше не нужна free_string
}
