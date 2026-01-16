#include "mainwindow.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QDebug>

#include "../hpps/postgres_repository.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //sql drivers
    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

    // 2)connect to sql
    PostgresRepository pg(
        "localhost",
        5432,
        "phonebook",
        "phonebook_user",
        "Nikorlov08"
    );

    const auto list = pg.loadAll();
    qDebug() << "Loaded from PG:" << (int)list.size();

    //gui 
    MainWindow w;
    w.show();

    return a.exec();
}
