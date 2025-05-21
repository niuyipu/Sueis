
/*void MainWindow::createUser(const QString &username, const QString &password)

    QMap<QString, QStringList> defaultCategories = {
        { QStringLiteral("收入"),       { QStringLiteral("工资／薪金"), QStringLiteral("奖金"), QStringLiteral("投资收益"), QStringLiteral("兼职收入"), QStringLiteral("礼金") } },
        { QStringLiteral("餐饮"),       { QStringLiteral("早餐"), QStringLiteral("午餐"), QStringLiteral("晚餐"), QStringLiteral("夜宵"), QStringLiteral("零食"), QStringLiteral("饮料") } },
        { QStringLiteral("交通出行"),    { QStringLiteral("公交／地铁"), QStringLiteral("打车"), QStringLiteral("私车油费"), QStringLiteral("停车费"), QStringLiteral("高铁／火车票"), QStringLiteral("机票") } },
        { QStringLiteral("住房"),       { QStringLiteral("房租／按揭"), QStringLiteral("物业费"), QStringLiteral("水费"), QStringLiteral("电费"), QStringLiteral("燃气费"), QStringLiteral("网络费") } },
        { QStringLiteral("通信"),       { QStringLiteral("手机话费"), QStringLiteral("宽带费"), QStringLiteral("邮寄快递费") } },
        { QStringLiteral("日常购物"),    { QStringLiteral("服饰鞋包"), QStringLiteral("超市购物"), QStringLiteral("家居用品"), QStringLiteral("美妆护肤") } },
        { QStringLiteral("娱乐休闲"),    { QStringLiteral("电影／演出"), QStringLiteral("旅游／酒店"), QStringLiteral("健身／运动"), QStringLiteral("游戏／动漫"), QStringLiteral("聚会") } },
        { QStringLiteral("教育学习"),    { QStringLiteral("书籍"), QStringLiteral("培训课程"), QStringLiteral("考试报名费"), QStringLiteral("线上课程") } },
        { QStringLiteral("医疗健康"),    { QStringLiteral("医药费"), QStringLiteral("体检费"), QStringLiteral("门诊／住院"), QStringLiteral("保险支出") } },
        { QStringLiteral("理财投资"),    { QStringLiteral("基金／股票"), QStringLiteral("P2P／互联网理财"), QStringLiteral("定期存款") } },
        { QStringLiteral("其他支出"),    { QStringLiteral("捐赠／公益"), QStringLiteral("手续费"), QStringLiteral("宠物开销"), QStringLiteral("杂项") } }
    };

    QJsonArray categoryArray;
    for (auto it = defaultCategories.constBegin(); it != defaultCategories.constEnd(); ++it) {
        QJsonObject majorObj;
        majorObj.insert("name", it.key());
        QJsonArray subArr;
        for (const QString &sub : it.value())
            subArr.append(sub);
        majorObj.insert("sub", subArr);
        categoryArray.append(majorObj);
    }

    QJsonObject rootObj;
    rootObj.insert("categories", categoryArray);

    QJsonDocument catDoc(rootObj);

    QFile fCategory(path + "/category.json");
    if (fCategory.open(QIODevice::WriteOnly)) {
        fCategory.write(catDoc.toJson(QJsonDocument::Indented));
        fCategory.close();
    }
*/




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


/*void MainWindow::loadCategories(const QString &username) {

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
}*/


/*
void MainWindow::onImportBill() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择账单文件", QString(), "Excel 文件 (*.xlsx *.xls *.csv)");
    if (fileName.isEmpty()) return;

    ui->labelSelectedFile->setText(fileName);
    ui->pages->setCurrentIndex(6);


    QXlsx::Document xlsx(fileName);
    int rowCount = xlsx.dimension().rowCount();
    int colCount = xlsx.dimension().columnCount();
    previewModel = new QStandardItemModel(this);
    QStringList headers;
    /*for (int c = 1; c <= colCount; ++c) {
        headers << xlsx.cellAt(1, c)->value().toString();
    }
    if (rowCount == 0 || colCount == 0) {
        QMessageBox::warning(this, "错误", "无法读取 Excel 文件，可能文件格式有问题！");
        return;
    }

    previewModel = new QStandardItemModel(this);

    for (int c = 1; c <= colCount; ++c) {
        auto cellPtr = xlsx.cellAt(1, c);
        headers << (cellPtr ? cellPtr->value().toString() : "");
    }


    previewModel->setHorizontalHeaderLabels(headers);

    for (int r = 2; r <= rowCount; ++r) {
        QList<QStandardItem*> items;
        for (int c = 1; c <= colCount; ++c) {
            QVariant v = xlsx.cellAt(r, c)->value();
            items << new QStandardItem(v.toString());
        }
        previewModel->appendRow(items);
    }
    for (int r = 2; r <= rowCount; ++r) {
        QList<QStandardItem*> items;
        for (int c = 1; c <= colCount; ++c) {
            auto cellPtr = xlsx.cellAt(r, c);
            items << new QStandardItem(cellPtr ? cellPtr->value().toString() : "");
        }
        previewModel->appendRow(items);
    }

    ui->tableImportPreview->setModel(previewModel);
    ui->tableImportPreview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui->tableImportPreview->setModel(nullptr);

    ui->comboTargetAccount->clear();
    for (int i = 0; i < ui->listAccounts->count(); ++i)
        ui->comboTargetAccount->addItem(ui->listAccounts->item(i)->text());
    if (ui->comboTargetAccount->count() > 0)
        ui->comboTargetAccount->setCurrentIndex(0);
}*/

/*
void MainWindow::onImportBill() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择账单文件",
                                                    QString(),
                                                    "Excel 文件 (*.xlsx *.xls *.csv)");
    if (fileName.isEmpty()) return;

    ui->labelSelectedFile->setText(fileName);
    ui->pages->setCurrentIndex(6);

    QXlsx::Document xlsx(fileName);
    int rowCount = xlsx.dimension().rowCount();
    int colCount = xlsx.dimension().columnCount();
    if (rowCount < 2 || colCount == 0) {
        QMessageBox::warning(this, "错误", "无法读取 Excel 文件，可能文件格式有问题！");
        return;
    }
    QVector<int> validCols;
    QStringList headers;
    for (int c = 1; c <= colCount; ++c) {
        auto cell = xlsx.cellAt(1, c);
        QString h = cell ? cell->value().toString().trimmed() : QString();
        if (!h.isEmpty()) {
            validCols.append(c);
            headers << h;
        }
    }

    previewModel = new QStandardItemModel(this);
    previewModel->setHorizontalHeaderLabels(headers);

    for (int r = 2; r <= rowCount; ++r) {
        QList<QStandardItem*> items;
        bool nonEmpty = false;
        for (int idx = 0; idx < validCols.size(); ++idx) {
            int c = validCols[idx];
            auto cell = xlsx.cellAt(r, c);
            QString text = cell ? cell->value().toString().trimmed() : QString();
            items << new QStandardItem(text);
            if (!text.isEmpty()) nonEmpty = true;
        }
        if (nonEmpty) {
            previewModel->appendRow(items);
        }
    }

    ui->tableImportPreview->setModel(previewModel);
    ui->tableImportPreview->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);

    ui->comboTargetAccount->clear();
    for (int i = 0; i < ui->listAccounts->count(); ++i) {
        ui->comboTargetAccount->addItem(ui->listAccounts->item(i)->text());
    }
    if (ui->comboTargetAccount->count() > 0)
        ui->comboTargetAccount->setCurrentIndex(0);
}

*/

/*
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QMessageBox>
#include <QtSql>
#include <QFileDialog>

#include "xlsxdocument.h"

using namespace QXlsx;

static QDate excelBaseDate() { return QDate(1899, 12, 30); }
const QStringList dateKeywords = { "日", "日期", "Date", "date" };

void MainWindow::onImportBill() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择账单文件",
                                                    QString(),
                                                    "Excel 文件 (*.xlsx *.xls *.csv)");
    if (fileName.isEmpty()) return;

    ui->labelSelectedFile->setText(fileName);
    ui->pages->setCurrentIndex(6);

    QXlsx::Document xlsx(fileName);
    int rowCount = xlsx.dimension().rowCount();
    int colCount = xlsx.dimension().columnCount();
    if (rowCount < 2 || colCount == 0) {
        QMessageBox::warning(this, "错误", "无法读取 Excel 文件，可能文件格式有问题！");
        return;
    }

    // --- 找出有效列（表头非空） ---
    QVector<int> validCols;
    QStringList headers;
    for (int c = 1; c <= colCount; ++c) {
        auto cell = xlsx.cellAt(1, c);
        QString h = cell ? cell->value().toString().trimmed()
                         : QString();
        if (!h.isEmpty()) {
            validCols.append(c);
            headers << h;
        }
    }

    // 设置预览模型
    previewModel = new QStandardItemModel(this);
    previewModel->setHorizontalHeaderLabels(headers);

    // --- 读取数据行，去除全空行，日期序列号转换 ---
    for (int r = 2; r <= rowCount; ++r) {
        QList<QStandardItem*> items;
        bool nonEmpty = false;
        for (int idx = 0; idx < validCols.size(); ++idx) {
            int c = validCols[idx];
            auto cell = xlsx.cellAt(r, c);
            QString text;
            if (cell) {
                QVariant v = cell->value();
                // 如果是数字，且该列是“日期”标题，就做序列号转日期
                if (v.type() == QVariant::Double &&(
                    headers[idx].contains("日")||headers[idx].contains("时"))) {
                    double serial = v.toDouble();
                    QDate dt = excelBaseDate().addDays(int(serial));
                    text = dt.isValid()
                               ? dt.toString("yyyy-MM-dd")
                               : QString();
                } else {
                    text = v.toString().trimmed();
                }
            }
            items << new QStandardItem(text);
            if (!text.isEmpty()) nonEmpty = true;
        }
        if (nonEmpty) {
            previewModel->appendRow(items);
        }
    }

    ui->tableImportPreview->setModel(previewModel);
    ui->tableImportPreview->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);

    // 填充目标账单下拉
    ui->comboTargetAccount->clear();
    for (int i = 0; i < ui->listAccounts->count(); ++i) {
        ui->comboTargetAccount->addItem(
            ui->listAccounts->item(i)->text()
            );
    }
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
        } else {
            showDashboard();
        }
        return;
    }

    QString target = ui->comboTargetAccount->currentText();
    QString dbPath = QDir::currentPath() + "/users/" + currentUser
                     + "/" + target + ".db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", target + "_import");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        QMessageBox::critical(this, "错误", "无法打开目标数据库！");
        return;
    }

    QSqlTableModel model(this, db);
    model.setTable("transactions");
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.select();

    int rowCount = previewModel->rowCount();
    int colCount = previewModel->columnCount();

    // 取出“日期”列在 previewModel 中的索引
    int dateCol = -1;
    for (int i = 0; i < colCount; ++i) {
        if (previewModel->headerData(i, Qt::Horizontal)
                .toString().contains("日" 或 Date )) {
            dateCol = i;
            break;
        }
    }

    for (int r = 0; r < rowCount; ++r) {
        // 整行空白跳过
        bool nonEmpty = false;
        for (int c = 0; c < colCount; ++c) {
            if (!previewModel->item(r, c)->text().isEmpty()) {
                nonEmpty = true;
                break;
            }
        }
        if (!nonEmpty) continue;

        QSqlRecord rec = model.record();
        for (int c = 0; c < colCount; ++c) {
            QString header = previewModel->headerData(c, Qt::Horizontal)
            .toString();
            QString txt = previewModel->item(r, c)
                              ->text().trimmed();
            // 日期列
            if (c == dateCol) {
                // 若已经是 yyyy-MM-dd 格式，直接用
                QDate dt = QDate::fromString(txt, "yyyy-MM-dd");
                if (!dt.isValid() && !txt.isEmpty()) {
                    // 尝试转 number → date
                    double serial = txt.toDouble();
                    dt = excelBaseDate().addDays(int(serial));
                }
                rec.setValue("date", dt);
            }
            // 收支、金额等按原列名映射：
            else if (header.contains("备注")) {
                rec.setValue("note", txt);
            } else if (header.contains("收")||header.contains("出")) {
                rec.setValue("outin", txt);
            } else if (header.contains("金")) {
                rec.setValue("amount", txt.toDouble());
            }
            // …若有其它列需映射，再按列名判断 …
        }
        model.insertRecord(-1, rec);
    }

    if (!model.submitAll()) {
        QMessageBox::warning(this, "导入失败", model.lastError().text());
    } else {
        updateCurrentBalance();
        auto matches = ui->listAccounts->findItems(target, Qt::MatchExactly);
        if (!matches.isEmpty())
            ui->listAccounts->setCurrentItem(matches.first());
        onEditAccount();
    }

    QSqlDatabase::removeDatabase(target + "_import");
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
    QString dbPath = QDir::currentPath() + "/users/" + currentUser + "/" + target + ".db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", target + "_import");
    db.setDatabaseName(dbPath);
    db.open();
    QSqlTableModel model(this, db);
    model.setTable("transactions");
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.select();

    for (int r = 0; r < previewModel->rowCount(); ++r) {
        QSqlRecord rec = model.record();
        rec.setValue("date",     previewModel->item(r, 0)->text());
        rec.setValue("note",     previewModel->item(r, 1)->text());
        rec.setValue("outin", previewModel->item(r, 4)->text());
        rec.setValue("amount",   previewModel->item(r, 5)->text().toDouble());
        model.insertRecord(-1, rec);
    }
    model.submitAll();

    updateCurrentBalance();
    QList<QListWidgetItem*> matches = ui->listAccounts->findItems(target, Qt::MatchExactly);

    if (!matches.isEmpty()) {
        ui->listAccounts->setCurrentItem(matches.first());
    }

    onEditAccount();
    QSqlDatabase::removeDatabase(target + "_import");

}
*/

/*void MainWindow::updateCurrentBalance() {
    QSqlQuery q(QSqlDatabase::database(currentAccount));
    q.exec("SELECT SUM(amount) FROM transactions");
    currentBalance = q.next() ? q.value(0).toDouble() : 0.0;
    ui->labelCurrentAccountBalance->setText(
        QString("结余: %1").arg(currentBalance)
        );
}*/



    /*void MainWindow::onAddTransaction()
      QString jsonPath = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile f(jsonPath);
    QStringList categories;
    if (f.open(QIODevice::ReadOnly)) {QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        QJsonArray arr = doc.array(); for (const QJsonValue &v : arr) { if (v.isString()) {categories.append(v.toString()); } } f.close();
    } else {QMessageBox::warning(this, "错误", "无法加载类别文件：" + f.errorString());return;}*/



//connect(ui->btnDeleteCategory, &QPushButton::clicked,  this,  &MainWindow::onRemoveCategory);
//connect(ui->btnLogin2,  &QPushButton::clicked, this, [=](){onConfirmCreate(),onLogin();});



/*oneditaccount
 * QSqlQuery q(db);
    q.exec("SELECT SUM(amount) FROM transactions");
    double balance = 0;
    if (q.next()) balance = q.value(0).toDouble();
    ui->labelCurrentAccountBalance->setText(
        QString("结余: %1").arg(balance));
    */
