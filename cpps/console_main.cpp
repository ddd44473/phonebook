#include <iostream>
#include <limits>      // std::numeric_limits
#include <string>
#include <vector>

#include "../hpps/service.h"

// helper: read whole line
static void readLine(const std::string& prompt, std::string& out)
{
    std::cout << prompt;
    std::getline(std::cin, out);
}

static std::string phoneTypeToString(PhoneType t)
{
    switch (t) {
        case PhoneType::Work:    return "Work";
        case PhoneType::Home:    return "Home";
        case PhoneType::Service: return "Service";
    }
    return "Unknown";
}

int main()
{
    ContactService service("contacts.txt");

    while (true) {
        std::cout << "\n==== PHONE BOOK ====\n";
        std::cout << "1. Add contact\n";
        std::cout << "2. List contacts\n";
        std::cout << "3. Delete contact by id\n";
        std::cout << "4. Edit contact by id\n";
        std::cout << "0. Exit\n";
        std::cout << "Select: ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Try again.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) {
            std::cout << "Goodbye!\n";
            break;
        }

        if (choice == 1) {
            // --- Add contact ---
            std::string firstname, lastname, middlename;
            std::string address, birthday, email;
            std::string phoneNumber;

            readLine("First name: ", firstname);
            readLine("Last name: ", lastname);
            readLine("Middle name (optional): ", middlename);
            readLine("Address (optional): ", address);
            readLine("Birth date (YYYY-MM-DD, optional): ", birthday);
            readLine("Email: ", email);
            readLine("Phone number: ", phoneNumber);

            PhoneNumber p;
            p.type   = PhoneType::Work;
            p.number = phoneNumber;

            std::vector<PhoneNumber> phones;
            phones.push_back(p);

            std::string error;
            bool ok = service.createContact(
                firstname, lastname, middlename,
                address, birthday, email,
                phones, error
            );

            if (!ok) std::cout << "Error: " << error << "\n";
            else     std::cout << "Contact created.\n";
        }
        else if (choice == 2) {
            // --- List contacts ---
            const auto& all = service.getAll();
            if (all.empty()) {
                std::cout << "No contacts.\n";
            } else {
                std::cout << "\nContacts:\n";
                for (const auto& c : all) {
                    std::cout << "-------------------------\n";
                    std::cout << "ID: " << c.id << "\n";
                    std::cout << "Name: " << c.firstname << " " << c.lastname << "\n";
                    if (!c.middlename.empty()) std::cout << "Middle: " << c.middlename << "\n";
                    if (!c.address.empty())    std::cout << "Address: " << c.address << "\n";
                    if (!c.birthday.empty())   std::cout << "Birth: " << c.birthday << "\n";
                    std::cout << "Email: " << c.email << "\n";

                    std::cout << "Phones:\n";
                    for (const auto& p : c.phones) {
                        std::cout << "  [" << phoneTypeToString(p.type) << "] " << p.number << "\n";
                    }
                }
                std::cout << "-------------------------\n";
            }
        }
        else if (choice == 3) {
            // --- Delete by id ---
            std::cout << "Enter id to delete: ";
            int id;
            if (!(std::cin >> id)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid id.\n";
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (service.deleteById(id)) std::cout << "Contact deleted.\n";
            else                        std::cout << "Contact with this id not found.\n";
        }
        else if (choice == 4) {
            // --- Edit by id ---
            std::cout << "Enter id to edit: ";
            int id;
            if (!(std::cin >> id)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid id.\n";
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::string firstname, lastname, middlename;
            std::string address, birthday, email;
            std::string phoneNumber;

            readLine("First name: ", firstname);
            readLine("Last name: ", lastname);
            readLine("Middle name (optional): ", middlename);
            readLine("Address (optional): ", address);
            readLine("Birth date (YYYY-MM-DD, optional): ", birthday);
            readLine("Email: ", email);
            readLine("Phone number: ", phoneNumber);

            PhoneNumber p;
            p.type   = PhoneType::Work;
            p.number = phoneNumber;

            std::vector<PhoneNumber> phones;
            phones.push_back(p);

            std::string error;
            bool ok = service.editContact(
                id,
                firstname, lastname, middlename,
                address, birthday, email,
                phones,
                error
            );

            if (!ok) std::cout << "Error: " << error << "\n";
            else     std::cout << "Contact updated.\n";
        }
        else {
            std::cout << "Unknown option. Try again.\n";
        }
    }

    return 0;
}