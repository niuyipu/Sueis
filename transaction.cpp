#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CategoryDelegate.h"

#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>
#include <QtSql>


void MainWindow::onAddTransaction() {
    bool ok;

    QString jsonPath = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("错误"), tr("无法加载类别文件：%1").arg(f.errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isObject()) {
        QMessageBox::warning(this, tr("错误"), tr("类别文件格式不正确"));
        return;
    }
    QJsonObject rootObj = doc.object();
    QJsonArray categoryArr = rootObj.value("categories").toArray();

    QStringList majorList;
    for (const QJsonValue &val : categoryArr) {
        if (val.isObject()) {
            QString majorName = val.toObject().value("name").toString();
            majorList.append(majorName);
        }
    }

    double amt = QInputDialog::getDouble(this, "添加交易", "金额（不能为负）:", 0, 0, 1e9, 2, &ok);
    if (!ok) return;

    QStringList outins = { "收入", "支出" };
    QString major = QInputDialog::getItem(
        this, tr("选择大类"), tr("请选择大类："),majorList,0, false,  &ok);
    if (!ok) return;
    QStringList subList;
    for (const QJsonValue &val : categoryArr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        if (obj.value("name").toString() == major) {
            QJsonArray subs = obj.value("sub").toArray();
            for (const QJsonValue &s : subs) {
                if (s.isString()) subList.append(s.toString()); }break; }}
    QString category;
    if (subList.isEmpty()) { category = major;
    } else {category = QInputDialog::getItem( this,tr("选择小类"),
                                         tr("请选择“%1”的小类：").arg(major),  subList, 0, false,&ok );if (!ok) return;
        category = major + "/" + category;}qDebug() << "选择的类别是：" << category;

    QString outin= QInputDialog::getItem(this,"收支","请选择收支：",outins,0,false);
    if (!ok) return;
    QString note = QInputDialog::getText(this, "备注", "请输入备注(选填):");
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

    QStringList months;
    for (int m = 1; m <= 12; ++m) { months << QString::number(m);}
    bool okMonth = false;
    QString monthStr = QInputDialog::getItem(
        this, tr("选择月份"),tr("请选择月份："),months,
        QDate::currentDate().month() - 1, false,&okMonth );
    if (!okMonth) return;
    int month = monthStr.toInt();

    int daysInMonth = QDate(year, month, 1).daysInMonth();
    QStringList days;
    for (int d = 1; d <= daysInMonth; ++d) {days << QString::number(d);}
    bool okDay = false;
    QString dayStr = QInputDialog::getItem( this, tr("选择日期"),tr("请选择日："),days,
                                           QDate::currentDate().day() - 1,false, &okDay); if (!okDay) return;
    int day = dayStr.toInt();

    QDate selected = QDate(year, month, day);
    QString date = selected.toString(Qt::ISODate);

    QSqlRecord rec = transactionModel->record();
    rec.setValue("date", date);
    rec.setValue("amount", amt);
    rec.setValue("category", category);
    rec.setValue("outin", outin);
    rec.setValue("note", note);

    transactionModel->insertRecord(-1, rec);
    transactionModel->submitAll();
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
    transactionModel->select();
    updateCurrentBalance();
    updateTotalBalance();
}

void MainWindow::updateCurrentBalance() {
    QSqlDatabase db = QSqlDatabase::database(currentAccount);
    QSqlQuery q(db);
    QString sql = QString("SELECT SUM(CASE WHEN outin = '收入' THEN amount ELSE -amount END) ""FROM transactions" );
    if (!q.exec(sql)) {
        qWarning() << "计算结余失败：" << q.lastError().text();
        currentBalance = 0.0;
    } else {
        if (q.next() && !q.value(0).isNull())
            currentBalance = q.value(0).toDouble();
        else
            currentBalance = 0.0;
    }
    ui->labelCurrentAccountBalance->setText(QString("结余: %1").arg(currentBalance));
}

void MainWindow::updateTotalBalance() {
    double total = 0.0;
    QDir dir(QDir::currentPath() + "/users/" + currentUser);

    for (const QString &file : dir.entryList({ "*.db" }, QDir::Files)) {
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

void MainWindow::onTransactionsChanged(const QModelIndex& topLeft,
                                       const QModelIndex& bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    updateCurrentBalance();
    updateTotalBalance();
}


void MainWindow::updateCategoryDelegate() {
    QString path = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开分类文件：" << path;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (doc.isNull()) {
        qWarning() << "分类文件解析失败：" << path;
        return;
    }
    QJsonArray categories = doc.object().value("categories").toArray();

    QAbstractItemDelegate* oldDelegate = ui->tableTransactions->itemDelegateForColumn(
        transactionModel->fieldIndex("category")
        );
    if (oldDelegate) delete oldDelegate;

    ui->tableTransactions->setItemDelegateForColumn(
        transactionModel->fieldIndex("category"),
        new CategoryDelegate(categories, this)
        );

  ui->tableTransactions->reset();
}



