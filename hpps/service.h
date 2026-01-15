#pragma once

#include <memory>
#include <string>
#include <vector>

#include "contact.h"
#include "repository.h"   // интерфейс IRepository

// Параметры подключения к PostgreSQL (для Задачи 3)
struct DbConfig {
    std::string host;
    int port = 5432;
    std::string dbName;
    std::string user;
    std::string password;
};

// Сервисный класс: вся логика работы с контактами.
// Хранит контакты в памяти и сохраняет их в выбранное хранилище (файл или PostgreSQL).
class ContactService {
public:
    // Режим 1: хранилище ФАЙЛ (как в Задаче 2)
    explicit ContactService(const std::string& filePath);

    // Режим 2: хранилище PostgreSQL (Задача 3)
    explicit ContactService(const DbConfig& cfg);

    // Получить все контакты (для отображения в GUI/CLI).
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
    void loadFromStorage(); // загрузить contacts из repo
    void saveToStorage();   // сохранить contacts в repo

private:
    std::unique_ptr<IRepository> repo; // может быть FileRepository или PostgresRepository
    std::vector<Contact> contacts;     // контакты в памяти
    int nextId = 1;                    // следующий id для нового контакта
};
