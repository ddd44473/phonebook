#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ContactService;
class AddEditContactDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnDelete_clicked();
    void on_btnSave_clicked();
    void on_btnLoad_clicked();
    void on_searchTextChanged(const QString& text);

private:
    Ui::MainWindow *ui;

    std::unique_ptr<ContactService> service;

    QString contactsFilePath() const;
    QString cellText(int row, int col) const;
    void setRowFromDialog(int row, AddEditContactDialog& dlg);

    void refreshTable();                
    void applySearchFilter(const QString& text);
};

#endif 
