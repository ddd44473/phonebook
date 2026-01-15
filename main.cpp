#include "mainwindow.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QDebug>

#include "../hpps/postgres_repository.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1) ТЕСТ: какие SQL-драйверы видит Qt
    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

    // 2) ТЕСТ: подключение к PostgreSQL и создание схемы (ensureSchema вызывается внутри loadAll)
    // ВАЖНО: вставь пароль от phonebook_user вместо "PASTE_PASSWORD_HERE"
    PostgresRepository pg(
        "localhost",
        5432,
        "phonebook",
        "phonebook_user",
        "Nikorlov08"
    );

    const auto list = pg.loadAll();
    qDebug() << "Loaded from PG:" << (int)list.size();

    // 3) Запуск GUI
    MainWindow w;
    w.show();

    return a.exec();
}
