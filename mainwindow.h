#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonArray>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <qsqltablemodel.h>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QObject>
#include <QDate>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateCategoryDelegate();

private slots:
    void showHomePage();
    void showDashboard();
    void onConfirmCreate();
    void onLogin();
    void onDeleteUser();
    void onSelectUser();
    void onConfirmLogin();
    void onLogout();
    void onBrowseUsers();
    void onAddAccount();
    void onRemoveAccount();

    void onEditAccount();

    void onAddTransaction();
    void onRemoveTransaction();

    void onImportBill();
    void onConfirmImport();

    void showeditcategory();
    void AddMajorCategory();
    void AddMinorCategory();
    void DeleteMajorCategory();
    void DeleteMinorCategory();
    void showAnalysisPage();

    void onApplyFilter();
    void onTransactionsChanged(const QModelIndex&, const QModelIndex&);


    void onSelectReportFolder();
    void onGenerateReport();



private:

    Ui::MainWindow* ui;

    QString selectedUser;
    QString currentUser;
    QString currentAccount;
    QSqlTableModel *transactionModel;
    QStandardItemModel *previewModel;

    bool isLoggedIn = false;

    void ensureUsersFolder();
    void createUser(const QString &username, const QString &password);
    bool verifyPassword(const QString &username, const QString &password);
    void enterDashboard(const QString &username);
    void loadAccounts(const QString &username);
    void saveAccounts(const QString &username, const QJsonArray &arr);
    void loadCategories(const QString &username);
    void saveCategories(const QString &username, const QJsonArray &arr);

    double currentBalance = 0.0;

    void updateCurrentBalance();
    void updateTotalBalance();
    void initFilterControls();
    void loadAccountOptions();

signals:
    void categoriesUpdated();
};



class ComboDelegate : public QStyledItemDelegate {
public:
    ComboDelegate(const QStringList &items, QObject *parent=nullptr)
        : QStyledItemDelegate(parent), m_items(items) {}
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &,
                          const QModelIndex &) const override {
        auto cb = new QComboBox(parent);
        cb->addItems(m_items);
        return cb;
    }
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override {
        QString v = index.model()->data(index).toString();
        static_cast<QComboBox*>(editor)->setCurrentText(v);
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override {
        auto cb = static_cast<QComboBox*>(editor);
        model->setData(index, cb->currentText());
    }
private:
    QStringList m_items;
};


#endif
