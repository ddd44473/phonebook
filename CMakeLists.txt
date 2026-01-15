cmake_minimum_required(VERSION 3.5)
project(PhoneBookQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt modules (для PostgreSQL нужен Sql)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)

# ---------------- GUI (Qt) ----------------
add_executable(PhoneBookQt WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    cpps/addeditcontactdialog.cpp
    hpps/addeditcontactdialog.h
    cpps/AddEditContactDialog.ui

    cpps/contact.cpp
    cpps/file_repository.cpp
    cpps/postgres_repository.cpp
    cpps/service.cpp
    cpps/validation.cpp

    # (необязательно перечислять .h, но можно оставить)
    hpps/postgres_repository.h
)

target_include_directories(PhoneBookQt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/hpps
    ${CMAKE_CURRENT_SOURCE_DIR}/cpps
)

# где искать .ui, если .cpp лежит в /cpps
set_property(TARGET PhoneBookQt PROPERTY AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/cpps"
)

target_link_libraries(PhoneBookQt PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql)

# ---------------- Console (CLI) ----------------
add_executable(PhoneBookConsole
    cpps/console_main.cpp
    cpps/contact.cpp
    cpps/file_repository.cpp
    cpps/postgres_repository.cpp   # <-- ВАЖНО: иначе undefined reference
    cpps/service.cpp               # service теперь использует PostgresRepository
    cpps/validation.cpp
)

target_include_directories(PhoneBookConsole PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/hpps
    ${CMAKE_CURRENT_SOURCE_DIR}/cpps
)

# file_repository.cpp использует QFile/QTextStream -> нужен Qt5::Core
# postgres_repository.cpp использует QtSql -> нужен Qt5::Sql
target_link_libraries(PhoneBookConsole PRIVATE Qt5::Core Qt5::Sql)
