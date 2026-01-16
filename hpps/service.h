#pragma once

#include <memory>
#include <string>
#include <vector>

#include "contact.h"
#include "repository.h"   // IRepository

// Параметры подключения к PostgreSQL (для Задачи 3)
struct DbConfig {
    std::string host;
    int port = 5432;
    std::string dbName;
    std::string user;
    std::string password;
};


class ContactService {
public:
    // file
    explicit ContactService(const std::string& filePath);

    // database
    explicit ContactService(const DbConfig& cfg);

    // get all contacts
    const std::vector<Contact>& getAll() const;

    bool createContact(
        const std::string& firstname,
        const std::string& lastname,
        const std::string& middlename,
        const std::string& address,
        const std::string& birthday,
        const std::string& email,
        const std::vector<PhoneNumber>& phones,
        std::string& errorMessage
    );

    bool deleteById(int id);

    bool editContact(
        int id,
        const std::string& firstname,
        const std::string& lastname,
        const std::string& middlename,
        const std::string& address,
        const std::string& birthday,
        const std::string& email,
        const std::vector<PhoneNumber>& phones,
        std::string& error
    );

private:
    void loadFromStorage(); // load contacts from repo
    void saveToStorage();   // save contacts в repo

private:
    std::unique_ptr<IRepository> repo; // FileRepository or PostgresRepository
    std::vector<Contact> contacts;     // contacts
    int nextId = 1;                    // next id
};
