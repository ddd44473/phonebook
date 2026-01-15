#include "../hpps/service.h"

#include "../hpps/file_repository.h"
#include "../hpps/postgres_repository.h"
#include "../hpps/validation.h"

ContactService::ContactService(const std::string& filePath)
{
    repo = std::make_unique<FileRepository>(filePath);
    loadFromStorage();
}

ContactService::ContactService(const DbConfig& cfg)
{
    repo = std::make_unique<PostgresRepository>(
        cfg.host, cfg.port, cfg.dbName, cfg.user, cfg.password
    );
    loadFromStorage();
}

void ContactService::loadFromStorage()
{
    contacts = repo->loadAll();

    int maxId = 0;
    for (const auto& c : contacts) {
        if (c.id > maxId) maxId = c.id;
    }
    nextId = maxId + 1;
}

void ContactService::saveToStorage()
{
    repo->saveAll(contacts);
}

const std::vector<Contact>& ContactService::getAll() const
{
    return contacts;
}

bool ContactService::createContact(
    const std::string& firstname,
    const std::string& lastname,
    const std::string& middlename,
    const std::string& address,
    const std::string& birthday,
    const std::string& email,
    const std::vector<PhoneNumber>& phones,
    std::string& errorMessage
)
{
    errorMessage.clear();

    if (!validateName(firstname)) { errorMessage = "Invalid first name"; return false; }
    if (!validateName(lastname))  { errorMessage = "Invalid last name";  return false; }
    if (!middlename.empty() && !validateName(middlename)) {
        errorMessage = "Invalid middle name"; return false;
    }
    if (!birthday.empty() && !validateBirthDate(birthday)) {
        errorMessage = "Invalid birth date"; return false;
    }

    const std::string cleanEmail = normalizeEmail(email);
    if (!validateEmail(cleanEmail)) { errorMessage = "Invalid email"; return false; }

    if (phones.empty()) { errorMessage = "Contact must have at least one phone number"; return false; }
    for (const auto& p : phones) {
        if (!validatePhone(p.number)) {
            errorMessage = "Invalid phone number: " + p.number;
            return false;
        }
    }

    Contact c;
    c.id = nextId++;
    c.firstname  = firstname;
    c.lastname   = lastname;
    c.middlename = middlename;
    c.address    = address;
    c.birthday   = birthday;
    c.email      = cleanEmail;
    c.phones     = phones;

    contacts.push_back(c);
    saveToStorage();
    return true;
}

bool ContactService::deleteById(int id)
{
    for (auto it = contacts.begin(); it != contacts.end(); ++it) {
        if (it->id == id) {
            contacts.erase(it);
            saveToStorage();
            return true;
        }
    }
    return false;
}

bool ContactService::editContact(
    int id,
    const std::string& firstname,
    const std::string& lastname,
    const std::string& middlename,
    const std::string& address,
    const std::string& birthday,
    const std::string& email,
    const std::vector<PhoneNumber>& phones,
    std::string& error
)
{
    error.clear();

    Contact* target = nullptr;
    for (auto& c : contacts) {
        if (c.id == id) { target = &c; break; }
    }
    if (!target) { error = "Contact not found"; return false; }

    if (!validateName(firstname)) { error = "Invalid first name"; return false; }
    if (!validateName(lastname))  { error = "Invalid last name";  return false; }
    if (!middlename.empty() && !validateName(middlename)) { error = "Invalid middle name"; return false; }

    const std::string cleanEmail = normalizeEmail(email);
    if (!validateEmail(cleanEmail)) { error = "Invalid email"; return false; }

    if (!birthday.empty() && !validateBirthDate(birthday)) { error = "Invalid birth date"; return false; }

    if (phones.empty()) { error = "At least 1 phone required"; return false; }
    for (const auto& p : phones) {
        if (!validatePhone(p.number)) {
            error = "Invalid phone number: " + p.number;
            return false;
        }
    }

    target->firstname  = firstname;
    target->lastname   = lastname;
    target->middlename = middlename;
    target->address    = address;
    target->birthday   = birthday;
    target->email      = cleanEmail;
    target->phones     = phones;

    saveToStorage();
    return true;
}
