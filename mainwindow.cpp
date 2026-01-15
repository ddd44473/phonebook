#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "addeditcontactdialog.h"
#include "../hpps/service.h"
#include "../hpps/validation.h"   // <-- ВАЖНО для validatePhone()

#include <QDialog>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

static QString sanitize(QString s)
{
    s.replace('\t', ' ');
    s.replace('\n', ' ');
    s.replace('\r', ' ');
    return s.trimmed();
}

// Разбиваем строку с телефонами по ';'  (можно: "8...; +7...; 8...")
static QStringList splitPhones(QString s)
{
    s = sanitize(s);
    if (s.isEmpty()) return {};

    QStringList parts = s.split(';', Qt::SkipEmptyParts);
    for (QString &p : parts) p = sanitize(p);
    return parts;
}

// Добавляем в out телефоны заданного типа из строки input.
// Валидируем каждый номер по validatePhone().
static bool addPhonesOfType(std::vector<PhoneNumber>& out, PhoneType type, const QString& input, std::string& err)
{
    const QStringList items = splitPhones(input);
    for (const QString& it : items) {
        const std::string num = it.toStdString();
        if (!validatePhone(num)) {
            err = "Некорректный телефон: " + num + "\n"
                  "Если номеров несколько — разделяй их через ';'.";
            return false;
        }
        out.push_back({type, num});
    }
    return true;
}

// Возвращаем строку со ВСЕМИ телефонами данного типа, через "; "
static QString phonesByTypeString(const Contact& c, PhoneType t)
{
    QStringList items;
    for (const auto& p : c.phones) {
        if (p.type == t) {
            items << QString::fromStdString(p.number);
        }
    }
    return items.join("; ");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidget->setSortingEnabled(true);

    ui->lineEdit->setPlaceholderText("Поиск...");
    ui->lineEdit->clear();

    // явные connect
    connect(ui->btnAdd,    &QPushButton::clicked, this, &MainWindow::on_btnAdd_clicked);
    connect(ui->btnEdit,   &QPushButton::clicked, this, &MainWindow::on_btnEdit_clicked);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::on_btnDelete_clicked);
    connect(ui->btnSave,   &QPushButton::clicked, this, &MainWindow::on_btnSave_clicked);
    connect(ui->btnLoad,   &QPushButton::clicked, this, &MainWindow::on_btnLoad_clicked);
    connect(ui->lineEdit,  &QLineEdit::textChanged, this, &MainWindow::on_searchTextChanged);

    connect(ui->btnUseFile, &QPushButton::clicked, this, [this]() {
        service = std::make_unique<ContactService>(contactsFilePath().toStdString());
        refreshTable();
        applySearchFilter(ui->lineEdit->text());
        QMessageBox::information(this, "Хранилище", "Теперь используется ФАЙЛ.");
    });

    connect(ui->btnUseDb, &QPushButton::clicked, this, [this]() {
        DbConfig cfg;
        cfg.host = "localhost";
        cfg.port = 5432;
        cfg.dbName = "phonebook";
        cfg.user = "phonebook_user";
        cfg.password = "Nikorlov08"; // <-- ты вставишь сам

        service = std::make_unique<ContactService>(cfg);
        refreshTable();
        applySearchFilter(ui->lineEdit->text());
        QMessageBox::information(this, "Хранилище", "Теперь используется PostgreSQL.");
    });

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    service = std::make_unique<ContactService>(contactsFilePath().toStdString());

    refreshTable();
    applySearchFilter(ui->lineEdit->text());
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::contactsFilePath() const
{
    // AppData/Roaming/PhoneBookQt/contacts.txt
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return QDir(dir).filePath("contacts.txt");
}

QString MainWindow::cellText(int row, int col) const
{
    auto *it = ui->tableWidget->item(row, col);
    return it ? it->text() : QString();
}

void MainWindow::refreshTable()
{
    ui->tableWidget->setRowCount(0);

    const auto& all = service->getAll();

    for (const auto& c : all) {
        const int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        auto setItem = [&](int col, const QString& v){
            ui->tableWidget->setItem(row, col, new QTableWidgetItem(v));
        };

        // ПОРЯДОК КОЛОНОК (9 штук, индексы 0..8):
        // 0 Фамилия | 1 Имя | 2 Отчество | 3 Email |
        // 4 Рабочий | 5 Домашний | 6 Служебный |
        // 7 Дата рождения | 8 Адрес
        setItem(0, QString::fromStdString(c.lastname));
        setItem(1, QString::fromStdString(c.firstname));
        setItem(2, QString::fromStdString(c.middlename));
        setItem(3, QString::fromStdString(c.email));

        setItem(4, phonesByTypeString(c, PhoneType::Work));
        setItem(5, phonesByTypeString(c, PhoneType::Home));
        setItem(6, phonesByTypeString(c, PhoneType::Service));

        setItem(7, QString::fromStdString(c.birthday));
        setItem(8, QString::fromStdString(c.address));

        // сохраняем id в UserRole (на первой ячейке строки)
        ui->tableWidget->item(row, 0)->setData(Qt::UserRole, c.id);
    }
}

void MainWindow::on_btnAdd_clicked()
{
    AddEditContactDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    std::vector<PhoneNumber> phones;
    std::string err;

    // 3 поля, каждое может содержать "в любом количестве" через ';'
    if (!addPhonesOfType(phones, PhoneType::Work,    dlg.phoneWork(), err) ||
        !addPhonesOfType(phones, PhoneType::Home,    dlg.phoneHome(), err) ||
        !addPhonesOfType(phones, PhoneType::Service, dlg.phoneService(), err))
    {
        QMessageBox::warning(this, "Ошибка", QString::fromStdString(err));
        return;
    }

    std::string serviceErr;
    const bool ok = service->createContact(
        sanitize(dlg.firstName()).toStdString(),
        sanitize(dlg.lastName()).toStdString(),
        sanitize(dlg.middleName()).toStdString(),
        sanitize(dlg.address()).toStdString(),
        sanitize(dlg.birthDate()).toStdString(),
        sanitize(dlg.email()).toStdString(),
        phones,
        serviceErr
    );

    if (!ok) {
        QMessageBox::warning(this, "Ошибка", QString::fromStdString(serviceErr));
        return;
    }

    refreshTable();
    applySearchFilter(ui->lineEdit->text());
}

void MainWindow::on_btnEdit_clicked()
{
    const int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Редактирование", "Выбери контакт в таблице.");
        return;
    }

    auto *idItem = ui->tableWidget->item(row, 0);
    if (!idItem) {
        QMessageBox::warning(this, "Редактирование", "Не удалось получить строку.");
        return;
    }

    const int id = idItem->data(Qt::UserRole).toInt();
    if (id <= 0) {
        QMessageBox::warning(this, "Редактирование", "У контакта нет корректного id.");
        return;
    }

    AddEditContactDialog dlg(this);

    // Заполняем диалог из таблицы (строго по индексам колонок)
    if (auto *w = dlg.findChild<QLineEdit*>("leFirstName"))   w->setText(cellText(row, 1));
    if (auto *w = dlg.findChild<QLineEdit*>("leLastName"))    w->setText(cellText(row, 0));
    if (auto *w = dlg.findChild<QLineEdit*>("leOtch"))        w->setText(cellText(row, 2));
    if (auto *w = dlg.findChild<QLineEdit*>("leEmail"))       w->setText(cellText(row, 3));

    if (auto *w = dlg.findChild<QLineEdit*>("lePhoneWork"))    w->setText(cellText(row, 4));
    if (auto *w = dlg.findChild<QLineEdit*>("lePhoneHome"))    w->setText(cellText(row, 5));
    if (auto *w = dlg.findChild<QLineEdit*>("lePhoneService")) w->setText(cellText(row, 6));

    if (auto *w = dlg.findChild<QLineEdit*>("leBirthDate"))   w->setText(cellText(row, 7));
    if (auto *w = dlg.findChild<QLineEdit*>("leAddress"))     w->setText(cellText(row, 8));

    if (dlg.exec() != QDialog::Accepted)
        return;

    std::vector<PhoneNumber> phones;
    std::string err;

    if (!addPhonesOfType(phones, PhoneType::Work,    dlg.phoneWork(), err) ||
        !addPhonesOfType(phones, PhoneType::Home,    dlg.phoneHome(), err) ||
        !addPhonesOfType(phones, PhoneType::Service, dlg.phoneService(), err))
    {
        QMessageBox::warning(this, "Ошибка", QString::fromStdString(err));
        return;
    }

    std::string serviceErr;
    const bool ok = service->editContact(
        id,
        sanitize(dlg.firstName()).toStdString(),
        sanitize(dlg.lastName()).toStdString(),
        sanitize(dlg.middleName()).toStdString(),
        sanitize(dlg.address()).toStdString(),
        sanitize(dlg.birthDate()).toStdString(),
        sanitize(dlg.email()).toStdString(),
        phones,
        serviceErr
    );

    if (!ok) {
        QMessageBox::warning(this, "Ошибка", QString::fromStdString(serviceErr));
        return;
    }

    refreshTable();
    applySearchFilter(ui->lineEdit->text());
}

void MainWindow::on_btnDelete_clicked()
{
    const int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Удаление", "Выбери контакт в таблице.");
        return;
    }

    auto *idItem = ui->tableWidget->item(row, 0);
    if (!idItem) {
        QMessageBox::warning(this, "Удаление", "Не удалось получить строку.");
        return;
    }

    const int id = idItem->data(Qt::UserRole).toInt();
    if (id <= 0) {
        QMessageBox::warning(this, "Удаление", "У контакта нет корректного id.");
        return;
    }

    const auto answer = QMessageBox::question(
        this, "Удаление", "Удалить выбранный контакт?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (answer != QMessageBox::Yes)
        return;

    if (!service->deleteById(id)) {
        QMessageBox::warning(this, "Удаление", "Контакт не найден (возможна рассинхронизация).");
        return;
    }

    refreshTable();
    applySearchFilter(ui->lineEdit->text());
}

void MainWindow::on_btnSave_clicked()
{
    QMessageBox::information(this, "Сохранение",
        "Сохранение происходит автоматически при добавлении/редактировании/удалении контактов.");
}

void MainWindow::on_btnLoad_clicked()
{
    service = std::make_unique<ContactService>(contactsFilePath().toStdString());
    refreshTable();
    applySearchFilter(ui->lineEdit->text());

    QMessageBox::information(this, "Загрузка",
        "Загружено контактов: " + QString::number((int)service->getAll().size()) +
        "\nФайл:\n" + contactsFilePath());
}

void MainWindow::on_searchTextChanged(const QString& text)
{
    applySearchFilter(text);
}

void MainWindow::applySearchFilter(const QString& text)
{
    const QString needle = text.trimmed().toLower();
    auto *t = ui->tableWidget;

    for (int r = 0; r < t->rowCount(); ++r) {
        bool match = needle.isEmpty();

        if (!match) {
            for (int c = 0; c < t->columnCount(); ++c) {
                const QString v = cellText(r, c).toLower();
                if (v.contains(needle)) { match = true; break; }
            }
        }

        t->setRowHidden(r, !match);
    }
}
