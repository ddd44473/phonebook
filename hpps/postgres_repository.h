#pragma once

#include <string>
#include <vector>

#include "../hpps/contact.h"   // Contact, PhoneNumber, PhoneType
#include "../hpps/repository.h"

class PostgresRepository : public IRepository
{
public:
    PostgresRepository(
        const std::string& host,
        int port,
        const std::string& dbName,
        const std::string& user,
        const std::string& password
    );

    std::vector<Contact> loadAll() const override;
    void saveAll(const std::vector<Contact>& contacts) const override;

private:
    void ensureSchema() const;

private:
    std::string host_;
    int port_;
    std::string dbName_;
    std::string user_;
    std::string password_;
};
