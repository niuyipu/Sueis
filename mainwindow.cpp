// MainWindow.cpp
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "xlsxdocument.h"
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
#include <QFileDialog>

using namespace QXlsx;
// 构造与初始化
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ensureUsersFolder();
;
    // Home 页按钮
    connect(ui->btnCreate, &QPushButton::clicked, this, [=](){ showHomePage(); ui->pages->setCurrentIndex(1); });
    connect(ui->btnCreate2, &QPushButton::clicked, this, [=](){ showHomePage(); ui->pages->setCurrentIndex(1); });
    connect(ui->btnDeleteUser, &QPushButton::clicked,this, &MainWindow::onDeleteUser);
    connect(ui->btnLogin,  &QPushButton::clicked, this, [=](){onLogin();});
    connect(ui->btnLogin2,  &QPushButton::clicked, this, [=](){onConfirmCreate(),onLogin();});

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
    connect(ui->btnDashboard2, &QPushButton::clicked, this, &MainWindow::showDashboard);

    connect(ui->btnEditAccount, &QPushButton::clicked, this, &MainWindow::onEditAccount);

    connect(ui->btnAddTransaction, &QPushButton::clicked, this, &MainWindow::onAddTransaction);
    connect(ui->btnRemoveTransaction, &QPushButton::clicked, this, &MainWindow::onRemoveTransaction);

    connect(ui->btnImportBill,   &QPushButton::clicked,   this, &MainWindow::onImportBill);
    connect(ui->btnConfirmImport, &QPushButton::clicked,  this, &MainWindow::onConfirmImport);

    connect(ui->btneditcate, &QPushButton::clicked,  this,  &MainWindow::showeditcategory);
    connect(ui->btnAddCategory, &QPushButton::clicked,  this,  &MainWindow::onAddCategory);
    connect(ui->btnDeleteCategory, &QPushButton::clicked,  this,  &MainWindow::onRemoveCategory);
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
    updateTotalBalance();
}

void MainWindow::onLogin(){
    ui->listExistingUsers->clear();
    QDir d(QDir::currentPath() + "/users");
    ui->listExistingUsers->addItems(d.entryList(QDir::Dirs|QDir::NoDotAndDotDot));
    ui->pages->setCurrentIndex(2);
}

// 创建用户按钮
void MainWindow::onConfirmCreate() {
    QString user = ui->editNewUsername->text().trimmed();
    QString pwd  = ui->editNewPassword->text();
    if (user.isEmpty() ) {
        QMessageBox::warning(this, "错误", "用户名不能为空");
        return;
    }
    createUser(user, pwd);
    ui->editNewUsername->clear();
    ui->editNewPassword->clear();
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

    QStringList defaultCategories = { QStringLiteral("未分类") };
    QJsonArray catArr;
    for (const QString &cat : defaultCategories) {catArr.append(cat);}
    QJsonDocument catDoc(catArr);
    QFile fCategory(path + "/category.json");
    if (fCategory.open(QIODevice::WriteOnly)) {fCategory.write(catDoc.toJson());fCategory.close();}
    QMessageBox::information(this, "成功", "用户创建成功");
}

void MainWindow::onDeleteUser()
{
    // 确认当前用户
    auto item = ui->listExistingUsers->currentItem();
    if (!item) {
        QMessageBox::warning(this, "错误", "当前没有选中的用户");
        return;
    }
    selectedUser = item->text();
    QString username = selectedUser;

    // 二次确认
    auto ret = QMessageBox::question(
        this, "删除确认",
        QString("确定要删除用户 \"%1\" 吗？此操作会删除所有数据且不可恢复。").arg(username),
        QMessageBox::Yes | QMessageBox::No
        );
    if (ret != QMessageBox::Yes)
        return;

    // 删除用户目录及所有子文件
    QString userDirPath = QDir::currentPath() + "/users/" + username;
    QDir userDir(userDirPath);
    if (!userDir.exists()) {
        QMessageBox::warning(this, "错误", "用户目录不存在");
        return;
    }

    // 递归删除
    bool removed = true;
    for (const QFileInfo &info : userDir.entryInfoList(
             QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst)) {
        if (info.isDir()) {
            QDir dir(info.absoluteFilePath());
            if (!dir.removeRecursively()) {
                removed = false;
                qWarning() << "删除子目录失败：" << dir.path();
            }
        } else {
            if (!QFile::remove(info.absoluteFilePath())) {
                removed = false;
                qWarning() << "删除文件失败：" << info.absoluteFilePath();
            }
        }
    }
    // 最后删除用户根目录
    if (!userDir.rmdir(userDirPath)) {
        removed = false;
        qWarning() << "删除用户目录失败：" << userDirPath;
    }

    if (removed) {
        QMessageBox::information(this, "成功", "用户已删除");
    } else {
        QMessageBox::warning(this, "部分失败", "删除过程中出现错误，请检查日志");
    }
    onLogin();
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


//page3
// 进入 Dashboard
void MainWindow::enterDashboard(const QString &username) {
    isLoggedIn = true;
    currentUser = username;
    ui->labelWelcome->setText(QString("欢迎回来, %1").arg(username));
    loadAccounts(username);
    showDashboard();
    updateTotalBalance();
}

// 退出登录
void MainWindow::onLogout() {
    isLoggedIn = false;
    ui->editLoginPassword->clear();
    showHomePage();
}



//page4
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

    if (!ok) return;
    if (name.isEmpty() ) {
        QMessageBox::warning(this, "错误", "账户名不能为空");
        return;
    }
    // 校验：只允许中文、英文、数字
    QRegularExpression regex("^[\u4e00-\u9fa5a-zA-Z0-9]+$");

    if (!regex.match(name).hasMatch()) {
        QMessageBox::warning(this, "无效名称", "账户名称只能包含中文、字母和数字！");
        // 递归重试
        onAddAccount();
        return;
    }
    if (!ok || name.trimmed().isEmpty()) return;

    QString dbFile = name + ".db";
    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + dbFile;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", name);
    db.setDatabaseName(dbPath);
    db.open();
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS transactions ("
               "id INTEGER PRIMARY KEY, amount REAL, "
               "category TEXT,outin TEXT, note TEXT, date TEXT)");
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
/*
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
}*/
void MainWindow::onRemoveAccount()
{
    auto item = ui->listAccounts->currentItem();
    if (!item) return;
    QString name = item->text();

    // 二次确认
    auto ret = QMessageBox::question(this, "删除确认",
                                     QString("确定要删除账户 '%1' 吗？此操作不可撤销。").arg(name),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    // 读 JSON
    QString jsonPath = QDir::currentPath() + "/users/" + currentUser + "/accounts.json";
    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "打开 accounts.json 失败：" << jsonPath;
        return;
    }
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    f.close();

    // 过滤并记录要删的 db 文件名
    QJsonArray newArr;
    QString dbFile;
    for (auto v : arr) {
        auto obj = v.toObject();
        if (obj["name"].toString() != name) {
            newArr.append(obj);
        } else {
            dbFile = obj["file"].toString();
        }
    }

    // 1) 先关闭并移除数据库连接
    if (QSqlDatabase::contains(name)) {
        QSqlDatabase db = QSqlDatabase::database(name);
        if (db.isOpen()) db.close();
        QSqlDatabase::removeDatabase(name);
    }

    // 2) 删除 .db 文件
    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + dbFile;
    if (!QFile::remove(dbPath)) {
        qWarning() << "删除数据库文件失败：" << dbPath
                   << "，文件存在？" << QFile::exists(dbPath);
    }

    // 3) 更新 JSON 并刷新界面
    saveAccounts(currentUser, newArr);
    loadAccounts(currentUser);
}



//page5
void MainWindow::onEditAccount() {
    auto item = ui->listAccounts->currentItem();
    // 设置当前账户名称

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

    // 计算并显示当前账户结余
    updateCurrentBalance();
    /*QSqlQuery q(db);
    q.exec("SELECT SUM(amount) FROM transactions");
    double balance = 0;
    if (q.next()) balance = q.value(0).toDouble();
    ui->labelCurrentAccountBalance->setText(
        QString("结余: %1").arg(balance));

    ui->labelCurrentAccountName->setText(QString("账户：%1").arg(currentAccount));*/
    // 绑定到视图
    ui->tableTransactions->setModel(transactionModel);
    ui->tableTransactions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableTransactions->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableTransactions->setSelectionMode(QAbstractItemView::SingleSelection);
    int idCol = transactionModel->fieldIndex("id");
    ui->tableTransactions->hideColumn(idCol);

    // 切换页面
    ui->pages->setCurrentIndex(5);
}


//记账
void MainWindow::onAddTransaction() {
    bool ok;
    QString jsonPath = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile f(jsonPath);
    QStringList categories;
    if (f.open(QIODevice::ReadOnly)) {QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        QJsonArray arr = doc.array(); for (const QJsonValue &v : arr) { if (v.isString()) {categories.append(v.toString()); } } f.close();
    } else {QMessageBox::warning(this, "错误", "无法加载类别文件：" + f.errorString());return;}
    double amt = QInputDialog::getDouble(this, "添加交易", "金额（不能为负）:", 0, 0, 1e9, 2, &ok);
    if (!ok) return;
    QStringList outins = { "收入", "支出" };
    QString category = QInputDialog::getItem(this,tr("选择类别"),  tr("请选择类别："), categories, 0,false, &ok);
    if (!ok) return;
    QString outin= QInputDialog::getItem(this,"收支","请选择收支：",outins,0,false);
    if (!ok) return;
    QString note = QInputDialog::getText(this, "备注", "请输入备注(选填):");
    //QString date = QDate::currentDate().toString(Qt::ISODate);

    // 1. 年
    int currentYear = QDate::currentDate().year();
    QStringList years;
    for (int y = currentYear - 100; y <= currentYear; ++y) {
        years << QString::number(y);
    }
    bool okYear = false;
    QString yearStr = QInputDialog::getItem( this, tr("选择年份"), tr("请选择年份："),years,
        years.indexOf(QString::number(currentYear)),false,  &okYear );
    if (!okYear) return;
    int year = yearStr.toInt();

    // 2. 月
    QStringList months;
    for (int m = 1; m <= 12; ++m) { months << QString::number(m);}
    bool okMonth = false;
    QString monthStr = QInputDialog::getItem(
        this, tr("选择月份"),tr("请选择月份："),months,
        QDate::currentDate().month() - 1, false,&okMonth );
    if (!okMonth) return;
    int month = monthStr.toInt();

    // 3. 日 —— 要根据年/月算出当月天数
    int daysInMonth = QDate(year, month, 1).daysInMonth();
    QStringList days;
    for (int d = 1; d <= daysInMonth; ++d) {days << QString::number(d);}
    bool okDay = false;
    QString dayStr = QInputDialog::getItem( this, tr("选择日期"),tr("请选择日："),days,
        QDate::currentDate().day() - 1,false, &okDay); if (!okDay) return;
    int day = dayStr.toInt();

    // 4. 拼成 QDate
    QDate selected = QDate(year, month, day);
    // 然后转字符串或直接使用
    QString date = selected.toString(Qt::ISODate);

    QSqlRecord rec = transactionModel->record();
    rec.setValue("date", date);
    rec.setValue("amount", amt);
    rec.setValue("category", category);
    rec.setValue("outin", outin);
    rec.setValue("note", note);
    // ... 设置其他字段 ...
    transactionModel->insertRecord(-1, rec);
    transactionModel->submitAll();

    // 刷新模型与余额
    transactionModel->select();
    updateCurrentBalance();
    updateTotalBalance();

}

void MainWindow::onRemoveTransaction() {
    auto idx = ui->tableTransactions->currentIndex();

    if (!idx.isValid()) return;
    auto ret = QMessageBox::question(this, "删除确认",
                                     "确定要删除选中的交易吗？", QMessageBox::Yes|QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    transactionModel->removeRow(idx.row());
    transactionModel->submitAll();
    // 刷新模型与余额
    transactionModel->select();
    updateCurrentBalance();
    updateTotalBalance();
}

/*void MainWindow::updateCurrentBalance() {
    QSqlQuery q(QSqlDatabase::database(currentAccount));
    q.exec("SELECT SUM(amount) FROM transactions");
    currentBalance = q.next() ? q.value(0).toDouble() : 0.0;
    ui->labelCurrentAccountBalance->setText(
        QString("结余: %1").arg(currentBalance)
        );
}*/
void MainWindow::updateCurrentBalance() {
    // 获取当前数据库连接
    QSqlDatabase db = QSqlDatabase::database(currentAccount);
    QSqlQuery q(db);

    // SQL：如果“收支”列是“收入”，就加数额；否则减数额
    // 注意：SQLite 中标识符用双引号或反引号包裹中文列名
    QString sql = QString("SELECT SUM(CASE WHEN outin = '收入' THEN amount ELSE -amount END) ""FROM transactions" );
    if (!q.exec(sql)) {
        qWarning() << "计算结余失败：" << q.lastError().text();
        currentBalance = 0.0;
    } else {
        // 提取结果
        if (q.next() && !q.value(0).isNull())
            currentBalance = q.value(0).toDouble();
        else
            currentBalance = 0.0;
    }
    // 更新界面
    ui->labelCurrentAccountBalance->setText(QString("结余: %1").arg(currentBalance));
}

/*void MainWindow::updateTotalBalance() {
    double total = 0.0;
    QDir dir(QDir::currentPath() + "/users/" + currentUser);
    for (const QString &file : dir.entryList({"*.db"}, QDir::Files)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", file);
        db.setDatabaseName(dir.filePath(file));
        db.open();
        QSqlQuery q(db);
        q.exec("SELECT SUM(amount) FROM transactions");
        if (q.next()) total += q.value(0).toDouble();
        db.close();
        QSqlDatabase::removeDatabase(file);
    }
    ui->labelTotalBalance->setText(
        QString("总结余: %1").arg(total)
        );
}*/
void MainWindow::updateTotalBalance() {
    double total = 0.0;
    QDir dir(QDir::currentPath() + "/users/" + currentUser);

    for (const QString &file : dir.entryList({ "*.db" }, QDir::Files)) {
        // 打开一个临时连接，用文件名做连接名
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", file);
        db.setDatabaseName(dir.filePath(file));
        if (!db.open()) {
            qWarning() << "无法打开数据库" << file << ":" << db.lastError().text();
            QSqlDatabase::removeDatabase(file);
            continue;
        }
        QSqlQuery q(db);
        QString sql ="SELECT SUM(" " CASE WHEN outin = '收入' THEN amount "" ELSE -amount "" END"") FROM transactions";
        if (!q.exec(sql)) {
            qWarning() << "计算总余额失败(" << file << "):" << q.lastError().text();
        } else if (q.next() && !q.value(0).isNull()) {
            total += q.value(0).toDouble();
        }

        db.close();
        QSqlDatabase::removeDatabase(file);
    }

    ui->labelTotalBalance->setText(
        QString("总结余: %1").arg(total)
        );
}

//Page6

void MainWindow::onImportBill() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择账单文件", QString(), "Excel 文件 (*.xlsx)");
    if (fileName.isEmpty()) return;
    ui->labelSelectedFile->setText(fileName);
    ui->pages->setCurrentIndex(6);


    // 解析 Excel（需集成 QXlsx）
    QXlsx::Document xlsx(fileName);
    int rowCount = xlsx.dimension().rowCount();
    int colCount = xlsx.dimension().columnCount();
    previewModel = new QStandardItemModel(this);
    // 读取表头
    QStringList headers;
    for (int c = 1; c <= colCount; ++c) {
        headers << xlsx.cellAt(1, c)->value().toString();
    }
    previewModel->setHorizontalHeaderLabels(headers);
    // 读取数据行
    for (int r = 2; r <= rowCount; ++r) {
        QList<QStandardItem*> items;
        for (int c = 1; c <= colCount; ++c) {
            QVariant v = xlsx.cellAt(r, c)->value();
            items << new QStandardItem(v.toString());
        }
        previewModel->appendRow(items);
    }
    ui->tableImportPreview->setModel(previewModel);
    ui->tableImportPreview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui->tableImportPreview->setModel(nullptr);

    // 初始化账户下拉
    ui->comboTargetAccount->clear();
    for (int i = 0; i < ui->listAccounts->count(); ++i)
        ui->comboTargetAccount->addItem(ui->listAccounts->item(i)->text());
    if (ui->comboTargetAccount->count() > 0)
        ui->comboTargetAccount->setCurrentIndex(0);
}

void MainWindow::onConfirmImport() {

    if (ui->comboTargetAccount->count() == 0) {
        auto btn = QMessageBox::question(this, tr("无可用账户"),
                                         tr("当前没有可选账户，是否要先创建一个账户？"),
                                         QMessageBox::Yes | QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            showDashboard();
            onAddAccount();
            return;
        }
        else{showDashboard();return;}
    }

    QString target = ui->comboTargetAccount->currentText();
    // 打开目标账户数据库
    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + target + ".db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", target + "_import");
    db.setDatabaseName(dbPath);
    db.open();
    QSqlTableModel model(this, db);
    model.setTable("transactions");
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.select();

    // 假设预览表中列顺序为: date, amount, category, note
    for (int r = 0; r < previewModel->rowCount(); ++r) {
        QSqlRecord rec = model.record();
        rec.setValue("date",     previewModel->item(r, 0)->text());
        rec.setValue("amount",   previewModel->item(r, 5)->text().toDouble());
        rec.setValue("category", previewModel->item(r, 4)->text());
        rec.setValue("note",     previewModel->item(r, 1)->text());
        model.insertRecord(-1, rec);
    }
    model.submitAll();

    updateCurrentBalance();
    // 返回交易页面

    // 2. 在 listAccounts 中查找文本完全匹配的项
    QList<QListWidgetItem*> matches = ui->listAccounts->findItems(target, Qt::MatchExactly);

    // 3. 如果找到，就把第一个匹配项设为当前
    if (!matches.isEmpty()) {
        ui->listAccounts->setCurrentItem(matches.first());
    }

    onEditAccount();
    QSqlDatabase::removeDatabase(target + "_import");

}









void MainWindow::showeditcategory(){
    ui->pages->setCurrentIndex(7);
    loadCategories(selectedUser);
}

void MainWindow::loadCategories(const QString &username) {
    // 构造路径
    QString path = QDir::currentPath()+ "/users/" + username + "/category.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) { ui->listCategories->clear();return;}
    // 解析 JSON
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {ui->listCategories->clear(); return; }
    QJsonArray arr = doc.array();
    ui->listCategories->clear();
    for (const QJsonValue &v : arr) {
        if (v.isString()) {
            ui->listCategories->addItem(v.toString());
        }
    }
}


void MainWindow::saveCategories(const QString &username, const QJsonArray &arr) {
    QString path = QDir::currentPath() + "/users/" + username + "/category.json";
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
        f.close();
    }
}


void MainWindow::onAddCategory() {
    // 1. 弹出输入框，让用户输入类别名
    bool ok = false;
    QString name = QInputDialog::getText( this, tr("添加类别"), tr("类别名称："), QLineEdit::Normal, QString(),&ok);
    if (!ok) return;
    name = name.trimmed();
    if (name.isEmpty()) {QMessageBox::warning(this, tr("错误"), tr("类别名不能为空")); return; }

    // 2. 名称校验：只允许中文、英文字母、数字
    QRegularExpression regex(QStringLiteral("^[\u4e00-\u9fa5a-zA-Z0-9]+$"));
    if (!regex.match(name).hasMatch()) {
        QMessageBox::warning( this,  tr("无效名称"), tr("类别名称只能包含中文、字母和数字！") );  return; }

    // 3. 读取现有的 category.json
    QString path = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile file(path);
    QJsonArray arr;
    if (file.open(QIODevice::ReadOnly)) {QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) arr = doc.array(); file.close();}
    else { QDir().mkpath(QFileInfo(path).absolutePath());}

    // 4. 去重：如果已存在同名类别则提示并返回
    for (const QJsonValue &v : arr) {if (v.isString() && v.toString() == name) {
            QMessageBox::information( this, tr("提示"),  tr("该类别已存在") ); return; }
    }

    // 5. 追加新类别
    arr.append(name);

    // 6. 写回 JSON
    QJsonDocument outDoc(arr);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(outDoc.toJson(QJsonDocument::Indented));
        file.close();
    } else {QMessageBox::warning( this, tr("错误"), tr("无法写入类别文件：%1").arg(file.errorString())  ); return;}

    loadCategories(currentUser);
}


void MainWindow::onRemoveCategory()
{
    // 1. 选中检测 & 二次确认
    QListWidgetItem *item = ui->listCategories->currentItem();
    if (!item) return;

    QString name = item->text();
    if (QMessageBox::question(
            this,
            tr("删除确认"),
            tr("确定要删除类别“%1”吗？此操作不可撤销。").arg(name),
            QMessageBox::Yes | QMessageBox::No
            ) != QMessageBox::Yes)
    {
        return;
    }

    // 2. 读取 JSON（字符串数组）
    QString jsonPath = QDir::currentPath()
                       + "/users/" + currentUser
                       + "/category.json";

    QFile f(jsonPath);
    QJsonArray arr;
    if (f.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isArray()) arr = doc.array();
        f.close();
    } else {
        qWarning() << "打开 category.json 失败：" << f.errorString();
        return;
    }

    // 3. 过滤掉要删除的那个字符串
    QJsonArray newArr;
    for (const QJsonValue &v : arr) {
        if (v.isString() && v.toString() != name) {
            newArr.append(v);
        }
    }

    // 4. 保存到 JSON
    saveCategories(currentUser, newArr);

    // 5. 销毁对应的 DB 连接（如果你用了 name 作为连接名）
    if (QSqlDatabase::contains(name)) {
        QSqlDatabase db = QSqlDatabase::database(name);
        if (db.isOpen()) db.close();
        QSqlDatabase::removeDatabase(name);
    }

    // 6. 刷新界面
    loadCategories(currentUser);
}
