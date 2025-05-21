#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QtSql>

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

void MainWindow::saveAccounts(const QString &username, const QJsonArray &arr) {
    QString path = QDir::currentPath() + "/users/" + username + "/accounts.json";
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
        f.close();
    }
}


void MainWindow::onAddAccount() {
    bool ok;
    QString name = QInputDialog::getText(this, "添加账户", "账户名称：",
                                         QLineEdit::Normal, "", &ok);

    if (!ok) return;
    if (name.isEmpty() ) {
        QMessageBox::warning(this, "错误", "账户名不能为空");
        return;
    }

    QRegularExpression regex("^[\u4e00-\u9fa5a-zA-Z0-9]+$");

    if (!regex.match(name).hasMatch()) {
        QMessageBox::warning(this, "无效名称", "账户名称只能包含中文、字母和数字！");
        onAddAccount();
        return;
    }
    if (!ok || name.trimmed().isEmpty()) return;

    QString accountsPath = QDir::currentPath() + "/users/" + currentUser + "/accounts.json";
    QFile file(accountsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开账户列表文件！");
        return;
    }
    QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();
    file.close();

    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        if (obj["name"].toString() == name) {
            auto reply = QMessageBox::question(
                this,
                "账户已存在",
                QString("账户“%1”已存在，是否重新创建？").arg(name),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
                );
            if (reply == QMessageBox::Yes) {
                onAddAccount();
            }
            return;
        }
    }
    QString dbFile = name + ".db";
    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + dbFile;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", name);
    db.setDatabaseName(dbPath);
    db.open();
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS transactions ("
               "id INTEGER PRIMARY KEY, amount REAL, "
               "category TEXT,outin TEXT, note TEXT, date TEXT,"
               "counterpart TEXT)");
    db.close();
    QSqlDatabase::removeDatabase(name);

    QFile f(QDir::currentPath() + "/users/" + currentUser + "/accounts.json");
    f.open(QIODevice::ReadOnly);
    f.close();
    QJsonObject obj; obj["name"] = name; obj["file"] = dbFile;
    arr.append(obj);
    saveAccounts(currentUser, arr);
    loadAccounts(currentUser);
}

void MainWindow::onRemoveAccount()
{
    auto item = ui->listAccounts->currentItem();
    if (!item) return;
    QString name = item->text();

    auto ret = QMessageBox::question(this, "删除确认",
                                     QString("确定要删除账户 '%1' 吗？此操作不可撤销。").arg(name),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    QString jsonPath = QDir::currentPath() + "/users/" + currentUser + "/accounts.json";
    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "打开 accounts.json 失败：" << jsonPath;
        return;
    }
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    f.close();

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

    if (QSqlDatabase::contains(name)) {
        QSqlDatabase db = QSqlDatabase::database(name);
        if (db.isOpen()) db.close();
        QSqlDatabase::removeDatabase(name);
    }

    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + dbFile;
    if (!QFile::remove(dbPath)) {
        qWarning() << "删除数据库文件失败：" << dbPath
                   << "，文件存在？" << QFile::exists(dbPath);
    }

    saveAccounts(currentUser, newArr);
    loadAccounts(currentUser);
    updateTotalBalance();
}






void MainWindow::onEditAccount() {
    auto item = ui->listAccounts->currentItem();
    updateCategoryDelegate();

    if (!isLoggedIn) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }
    if (!item) {
        QMessageBox::warning(this, "提醒", "请先选择一个账户");
        return;
    }
    currentAccount = item->text();

    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + currentAccount + ".db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", currentAccount);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        QMessageBox::critical(this, "错误", "无法打开数据库：" + db.lastError().text());
        return;
    }

    transactionModel = new QSqlTableModel(this, db);
    transactionModel->setTable("transactions");
    //transactionModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    transactionModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    if (!transactionModel->select()) {
        QMessageBox::critical(this, "错误", "加载数据失败：" + transactionModel->lastError().text());
        return;
    }
    transactionModel->setHeaderData(
        transactionModel->fieldIndex("date"),
        Qt::Horizontal,
        QObject::tr("日期")
        );
    transactionModel->setHeaderData(
        transactionModel->fieldIndex("amount"),
        Qt::Horizontal,
        QObject::tr("金额")
        );
    transactionModel->setHeaderData(
        transactionModel->fieldIndex("category"),
        Qt::Horizontal,
        QObject::tr("类别")
        );
    transactionModel->setHeaderData(
        transactionModel->fieldIndex("outin"),
        Qt::Horizontal,
        QObject::tr("收入/支出")
        );
    transactionModel->setHeaderData(
        transactionModel->fieldIndex("note"),
        Qt::Horizontal,
        QObject::tr("备注")
        );

    ui->tableTransactions->setModel(transactionModel);
    updateCurrentBalance();
    ui->labelCurrentAccountName->setText(QString("账户：%1").arg(currentAccount));

    //ui->tableTransactions->setModel(transactionModel);

    // 允许双击和键盘直接编辑


    ui->tableTransactions->setModel(transactionModel);
    ui->tableTransactions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableTransactions->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableTransactions->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableTransactions->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed );
    connect(transactionModel,&QSqlTableModel::dataChanged,this,&MainWindow::onTransactionsChanged);


    int idCol = transactionModel->fieldIndex("id");
    ui->tableTransactions->hideColumn(idCol);
    ui->pages->setCurrentIndex(5);
}
