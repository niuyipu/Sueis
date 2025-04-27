// MainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonArray>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <qsqltablemodel.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 首页按钮
    void showHomePage();
    // 控制台
    void showDashboard();
    // 创建用户
    void onConfirmCreate();
    // 选择用户
    void onSelectUser();
    // 登录确认
    void onConfirmLogin();
    // 退出
    void onLogout();
    // 浏览 users 目录
    void onBrowseUsers();
    // 账户管理
    void onAddAccount();
    void onRemoveAccount();

    void onEditAccount();           // 跳转到账户编辑页面

    void onAddTransaction();
    void onRemoveTransaction();
private:
    Ui::MainWindow *ui;
    QString selectedUser;
    QString currentUser;
    QSqlTableModel *transactionModel;
    QString currentAccount; // 当前编辑的账户名
    bool isLoggedIn = false;

    void ensureUsersFolder();
    void createUser(const QString &username, const QString &password);
    bool verifyPassword(const QString &username, const QString &password);
    void enterDashboard(const QString &username);

    // 账户 JSON 操作
    void loadAccounts(const QString &username);
    void saveAccounts(const QString &username, const QJsonArray &arr);
};





#endif // MAINWINDOW_H
