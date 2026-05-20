#include "database.h"
#include "../core/catalog.h"
#include "../sql/parser.h"
#include "../sql/executor.h"
#include "../sql/result.h"

struct Database::Impl {
    std::unique_ptr<Catalog> catalog;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<Executor> executor;
    std::string current_db;
};

Database::Database() : pImpl(std::make_unique<Impl>()) {
    pImpl->parser = std::make_unique<Parser>();
}

Database::~Database() = default;

bool Database::open(const std::string& root_path) {
    try {
        pImpl->catalog = std::make_unique<Catalog>(root_path);
        pImpl->catalog->loadAll();
        pImpl->executor = std::make_unique<Executor>(pImpl->catalog.get());

        auto dbs = pImpl->catalog->listDatabases();
        if (!dbs.empty()) {
            pImpl->current_db = dbs[0];
            pImpl->executor->setCurrentDatabase(pImpl->current_db);
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void Database::close() {
    if (pImpl->catalog) {
        pImpl->catalog->saveAll();
    }
}

bool Database::useDatabase(const std::string& db_name) {
    if (!pImpl->catalog) return false;
    auto dbs = pImpl->catalog->listDatabases();
    for (const auto& db : dbs) {
        if (db == db_name) {
            pImpl->current_db = db_name;
            if (pImpl->executor) {
                pImpl->executor->setCurrentDatabase(db_name);
            }
            return true;
        }
    }
    return false;
}

std::vector<std::string> Database::listDatabases() const {
    if (!pImpl->catalog) return {};
    return pImpl->catalog->listDatabases();
}

ClientResult Database::execute(const std::string& sql) {
    try {
        auto stmt = pImpl->parser->parse(sql);
        if (!pImpl->executor) {
            pImpl->executor = std::make_unique<Executor>(pImpl->catalog.get());
        }
        pImpl->executor->setCurrentDatabase(pImpl->current_db);
        auto internalResult = pImpl->executor->execute(*stmt);
        
        pImpl->current_db = pImpl->executor->getCurrentDatabase();
        
        return ClientResult(internalResult);
    } catch (const std::exception& e) {
        return ClientResult::makeError(e.what());
    }
}