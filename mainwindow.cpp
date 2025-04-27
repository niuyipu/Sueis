// MainWindow.cpp
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QtSql>


// 构造与初始化
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ensureUsersFolder();
;

    // Home 页按钮
    connect(ui->btnCreate, &QPushButton::clicked, this, [=](){ showHomePage(); ui->pages->setCurrentIndex(1); });
    connect(ui->btnLogin,  &QPushButton::clicked, this, [=](){
        ui->listExistingUsers->clear();
        QDir d(QDir::currentPath() + "/users");
        ui->listExistingUsers->addItems(d.entryList(QDir::Dirs|QDir::NoDotAndDotDot));
        ui->pages->setCurrentIndex(2);
    });

    // Create User 页
    connect(ui->btnConfirmCreate, &QPushButton::clicked, this, &MainWindow::onConfirmCreate);

    // Select User 页
    connect(ui->btnSelectUser, &QPushButton::clicked, this, &MainWindow::onSelectUser);
    connect(ui->btnBrowseUsers, &QPushButton::clicked, this, &MainWindow::onBrowseUsers);

    // Login 页
    connect(ui->btnConfirmLogin, &QPushButton::clicked, this, &MainWindow::onConfirmLogin);

    // Dashboard 页
    connect(ui->btnLogout, &QPushButton::clicked, this, &MainWindow::onLogout);
    connect(ui->btnAddAccount, &QPushButton::clicked, this, &MainWindow::onAddAccount);
    connect(ui->btnRemoveAccount, &QPushButton::clicked, this, &MainWindow::onRemoveAccount);

    connect(ui->btnHome, &QPushButton::clicked, this, &MainWindow::showHomePage);
    connect(ui->btneditor, &QPushButton::clicked, this, &MainWindow::onEditAccount);
    connect(ui->btnDashboard, &QPushButton::clicked, this, &MainWindow::showDashboard);

    connect(ui->btnEditAccount, &QPushButton::clicked, this, &MainWindow::onEditAccount);

    connect(ui->btnAddTransaction, &QPushButton::clicked, this, &MainWindow::onAddTransaction);
    connect(ui->btnRemoveTransaction, &QPushButton::clicked, this, &MainWindow::onRemoveTransaction);

    // 首次显示 Home
    showHomePage();
}

MainWindow::~MainWindow() {
    delete ui;
}

// 确保 users 文件夹存在
void MainWindow::ensureUsersFolder() {
    QDir dir(QDir::currentPath() + "/users");
    if (!dir.exists()) dir.mkpath(dir.path());
}

// 显示首页
void MainWindow::showHomePage() {
    ui->pages->setCurrentIndex(0);
}

// 显示控制台(Dashboard)
void MainWindow::showDashboard() {
    if (!isLoggedIn) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }
    ui->pages->setCurrentIndex(4);
}

// 创建用户按钮
void MainWindow::onConfirmCreate() {
    QString user = ui->editNewUsername->text().trimmed();
    QString pwd  = ui->editNewPassword->text();
    if (user.isEmpty() || pwd.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }
    createUser(user, pwd);
}

// 实际创建用户逻辑
void MainWindow::createUser(const QString &username, const QString &password) {
    QString path = QDir::currentPath() + "/users/" + username;
    if (QDir(path).exists()) {
        QMessageBox::warning(this, "错误", "用户已存在");
        return;
    }
    QDir().mkpath(path);
    // 保存密码哈希
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QJsonObject obj; obj["password"] = QString(hash.toHex());
    QFile f(path + "/config.json");
    if (f.open(QIODevice::WriteOnly)) { f.write(QJsonDocument(obj).toJson()); f.close(); }
    // 初始化 accounts.json
    QFile f2(path + "/accounts.json");
    if (f2.open(QIODevice::WriteOnly)) { f2.write("[]"); f2.close(); }
    QMessageBox::information(this, "成功", "用户创建成功");
    showHomePage();
}

// 选择已有用户
void MainWindow::onSelectUser() {
    auto item = ui->listExistingUsers->currentItem();
    if (!item) {
        QMessageBox::warning(this, "错误", "请选择用户");
        return;
    }
    selectedUser = item->text();
    ui->labelLoginUser->setText(selectedUser);
    ui->pages->setCurrentIndex(3);
}

// 打开 users 文件夹
void MainWindow::onBrowseUsers() {
    QString usersPath = QDir::currentPath() + "/users";
    QDesktopServices::openUrl(QUrl::fromLocalFile(usersPath));
}

// 验证密码
bool MainWindow::verifyPassword(const QString &username, const QString &password) {
    QString cfg = QDir::currentPath() + "/users/" + username + "/config.json";
    QFile f(cfg); if (!f.open(QIODevice::ReadOnly)) return false;
    QString stored = QJsonDocument::fromJson(f.readAll()).object()["password"].toString();
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return stored == QString(hash.toHex());
}

// 登录确认
void MainWindow::onConfirmLogin() {
    QString pwd = ui->editLoginPassword->text();
    if (!verifyPassword(selectedUser, pwd)) {
        QMessageBox::warning(this, "错误", "用户名或密码错误");
        return;
    }
    enterDashboard(selectedUser);
}

// 进入 Dashboard
void MainWindow::enterDashboard(const QString &username) {
    isLoggedIn = true;
    currentUser = username;
    ui->labelWelcome->setText(QString("欢迎回来, %1").arg(username));
    loadAccounts(username);
    showDashboard();
}

// 退出登录
void MainWindow::onLogout() {
    isLoggedIn = false;
    ui->editLoginPassword->clear();
    showHomePage();
}




// 载入账户列表
void MainWindow::loadAccounts(const QString &username) {
    QString path = QDir::currentPath() + "/users/" + username + "/accounts.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    ui->listAccounts->clear();
    for (auto v : arr) {
        QString name = v.toObject()["name"].toString();
        ui->listAccounts->addItem(name);
    }
}

// 保存账户列表
void MainWindow::saveAccounts(const QString &username, const QJsonArray &arr) {
    QString path = QDir::currentPath() + "/users/" + username + "/accounts.json";
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
        f.close();
    }
}


// 添加账户
void MainWindow::onAddAccount() {
    bool ok;
    QString name = QInputDialog::getText(this, "添加账户", "账户名称：",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    QString dbFile = name + ".db";
    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + dbFile;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", name);
    db.setDatabaseName(dbPath);
    db.open();
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS transactions ("
               "id INTEGER PRIMARY KEY, amount REAL, "
               "category TEXT, note TEXT, date TEXT)");
    db.close();
    QSqlDatabase::removeDatabase(name);

    QFile f(QDir::currentPath() + "/users/" + currentUser + "/accounts.json");
    f.open(QIODevice::ReadOnly);
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    f.close();
    QJsonObject obj; obj["name"] = name; obj["file"] = dbFile;
    arr.append(obj);
    saveAccounts(currentUser, arr);
    loadAccounts(currentUser);
}

// 删除账户
void MainWindow::onRemoveAccount() {
    auto item = ui->listAccounts->currentItem();
    if (!item) return;
    QString name = item->text();


    // 二次确认，避免误删
    auto ret = QMessageBox::question(this, "删除确认",
        QString("确定要删除账户 '%1' 吗？此操作不可撤销。" ).arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;


    QString path = QDir::currentPath() + "/users/" + currentUser + "/accounts.json";
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    f.close();

    QJsonArray newArr;
    QString dbFile;
    for (auto v : arr) {
        if (v.toObject()["name"].toString() != name) {
            newArr.append(v);
        } else {
            dbFile = v.toObject()["file"].toString();
        }
    }
    QFile::remove(QDir::currentPath() + "/users/" + currentUser + "/" + dbFile);
    saveAccounts(currentUser, newArr);
    loadAccounts(currentUser);
}




void MainWindow::onEditAccount() {
    auto item = ui->listAccounts->currentItem();
    if (!isLoggedIn) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }
    if (!item) {
        QMessageBox::warning(this, "提醒", "请先选择一个账户");
        return;
    }
    currentAccount = item->text();

    // 打开对应 SQLite 数据库，使用账户名作为连接名
    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + currentAccount + ".db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", currentAccount);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        QMessageBox::critical(this, "错误", "无法打开数据库：" + db.lastError().text());
        return;
    }

    // 创建并配置模型
    transactionModel = new QSqlTableModel(this, db);
    transactionModel->setTable("transactions");
    transactionModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!transactionModel->select()) {
        QMessageBox::critical(this, "错误", "加载数据失败：" + transactionModel->lastError().text());
        return;
    }

    // 绑定到视图
    ui->tableTransactions->setModel(transactionModel);
    ui->tableTransactions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableTransactions->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableTransactions->setSelectionMode(QAbstractItemView::SingleSelection);

    // 切换页面
    ui->pages->setCurrentIndex(5);
}

void MainWindow::onAddTransaction() {
    bool ok;
    double amt = QInputDialog::getDouble(this, "添加交易", "金额 (正: 收入/负: 支出):", 0, -1e9, 1e9, 2, &ok);
    if (!ok) return;
    QString category = QInputDialog::getText(this, "分类", "请输入分类:");
    QString note = QInputDialog::getText(this, "备注", "请输入备注(选填):");
    QString date = QDate::currentDate().toString(Qt::ISODate);
    QSqlRecord rec = transactionModel->record();
    rec.setValue("amount", amt);
    rec.setValue("category", category);
    rec.setValue("note", note);
    rec.setValue("date", date);
    transactionModel->insertRecord(-1, rec);
    transactionModel->submitAll();
}

void MainWindow::onRemoveTransaction() {
    auto idx = ui->tableTransactions->currentIndex();
    if (!idx.isValid()) return;
    auto ret = QMessageBox::question(this, "删除确认",
                                     "确定要删除选中的交易吗？", QMessageBox::Yes|QMessageBox::No);
    if (ret != QMessageBox::Yes) return;
    transactionModel->removeRow(idx.row());
    transactionModel->submitAll();
}
