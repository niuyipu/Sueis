#include "mainwindow.h"
#include "ui_MainWindow.h"
#include <QDir>
#include<QDialog>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QTextStream>
#include <QJsonArray>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>

void MainWindow::showAnalysisPage(){
    ui->pages->setCurrentIndex(8);
    initFilterControls();
}


void MainWindow::initFilterControls()
{
    ui->comboType->addItems({ "全部", "收入", "支出" });
    loadAccountOptions();
    QString jsonPath = QDir::currentPath()
                       + "/users/" + currentUser
                       + "/category.json";
    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return;
    QJsonArray arr = doc.object().value("categories").toArray();
    ui->comboCategoryBig->addItem("全部");
    for (const QJsonValue &v : arr) {
        if (!v.isObject()) continue;
        ui->comboCategoryBig->addItem(
            v.toObject().value("name").toString());}
    connect(ui->comboCategoryBig, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int idx){
                ui->comboCategorySmall->clear();
                ui->comboCategorySmall->addItem("全部");
                if (idx == 0) return;
                auto obj = arr.at(idx-1).toObject();
                for (const QJsonValue &s : obj.value("sub").toArray()) {
                    ui->comboCategorySmall->addItem(s.toString()); }});
    ui->comboCategoryBig->setCurrentIndex(0);
    QDate earliest = QDate::currentDate();
    QDate latest = QDate::currentDate();
    int rows = transactionModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        QDate date = QDate::fromString(
            transactionModel->data(
                                transactionModel->index(i, transactionModel->fieldIndex("date"))
                                ).toString(),
            Qt::ISODate
            );
        if (date.isValid()) {
            if (date > latest) latest = date;
            if (date < earliest) earliest = date;
        }
    }
}



void MainWindow::onApplyFilter()
{
    QDate start = ui->dateEditStart->date();
    QDate end   = ui->dateEditEnd->date();
    QString bigCat   = ui->comboCategoryBig->currentText();
    QString smallCat = ui->comboCategorySmall->currentText();
    QString type     = ui->comboType->currentText();

    ui->tableFilterResult->clear();
    ui->tableFilterResult->setColumnCount(5);
    ui->tableFilterResult->setHorizontalHeaderLabels(
        { "日期", "大类", "小类", "金额", "备注" }
        );
    ui->tableFilterResult->setRowCount(0);

    double sumIncome = 0, sumExpense = 0;
    int rows = transactionModel->rowCount();
    int row  = 0;

    for (int i = 0; i < rows; ++i) {

        QDate  date     = QDate::fromString(
            transactionModel->data(
                                transactionModel->index(i,
                                                        transactionModel->fieldIndex("date"))
                                ).toString(),
            Qt::ISODate
            );
        QString catFull = transactionModel->data(
                                              transactionModel->index(i,
                                                                      transactionModel->fieldIndex("category"))
                                              ).toString();
        QStringList parts = catFull.split('/');
        QString big = parts.value(0);
        QString small = parts.size()>1 ? parts.at(1) : big;

        QString outin = transactionModel->data(
                                            transactionModel->index(i,
                                                                    transactionModel->fieldIndex("outin"))
                                            ).toString();
        double amount = transactionModel->data(
                                            transactionModel->index(i,
                                                                    transactionModel->fieldIndex("amount"))
                                            ).toDouble();
        QString note = transactionModel->data(
                                           transactionModel->index(i,
                                                                   transactionModel->fieldIndex("note"))
                                           ).toString();

        if (date < start || date > end)        continue;
        if (bigCat   != "全部" && big   != bigCat)   continue;
        if (smallCat != "全部" && small != smallCat) continue;
        if (type=="收入" && outin!="收入")    continue;
        if (type=="支出" && outin!="支出")    continue;

        ui->tableFilterResult->insertRow(row);
        ui->tableFilterResult->setItem(row, 0, new QTableWidgetItem(date.toString("yyyy-MM-dd")));
        ui->tableFilterResult->setItem(row, 1, new QTableWidgetItem(big));
        ui->tableFilterResult->setItem(row, 2, new QTableWidgetItem(small));
        ui->tableFilterResult->setItem(row, 3, new QTableWidgetItem(QString::number(amount, 'f', 2)));
        ui->tableFilterResult->setItem(row, 4, new QTableWidgetItem(note));
        ++row;

        if (outin == "收入")  sumIncome += amount;
        else                 sumExpense += amount;
    }

    double balance = sumIncome - sumExpense;
    ui->labelFilterBalance->setText(
        QString("收入：%1   支出：%2   结余：%3")
            .arg(sumIncome,0,'f',2)
            .arg(sumExpense,0,'f',2)
            .arg(balance,0,'f',2)
        );
}

void MainWindow::loadAccountOptions()
{
    ui->comboAccount->clear();
    ui->comboAccount->addItem("全部");

    QDir userDir(QDir::currentPath() + "/users/" + currentUser );
    QStringList accountFiles = userDir.entryList(QStringList() << "*.db", QDir::Files);
    for (const QString& fileName : accountFiles) {
        QString accountName = fileName.section(".", 0, 0);
        ui->comboAccount->addItem(accountName);
    }
}






void MainWindow::onSelectReportFolder()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "选择报表保存目录",
        QDir::currentPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->lblFolder->setText(dir);
    }
}
void MainWindow::onGenerateReport()
{
    qDebug() << "=== Report Generation Start ===";

    QDate start = ui->dateStart->date();
    QDate end   = ui->dateEnd->date();
    QString folder = ui->lblFolder->text().trimmed();
    QString fmt   = ui->comboFormat->currentText();
    qDebug() << "[Step 2] start =" << start.toString("yyyy-MM-dd")
             << ", end =" << end.toString("yyyy-MM-dd")
             << ", folder =" << folder
             << ", format =" << fmt;

    if (end < start) {
        QMessageBox::warning(this, "错误", "结束日期必须不早于开始日期");
        return;
    }
    if (folder.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先选择保存目录");
        return;
    }

    QString userDir = QDir::currentPath() + "/users/" + currentUser;
    QDir dir(userDir);
    QStringList dbFiles = dir.entryList(QStringList() << "*.db", QDir::Files);
    qDebug() << "[Step 3] Found DB files =" << dbFiles;
    if (dbFiles.isEmpty()) {
        QMessageBox::warning(this, "错误", "当前用户没有数据库文件");
        return;
    }

    QMap<QString, double> sumIncome, sumExpense;
    for (const QString &dbFile : dbFiles) {
        qDebug() << "[Step 4] Processing DB =" << dbFile;
        const QString connName = dbFile;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
            db.setDatabaseName(userDir + "/" + dbFile);
            if (!db.open()) {
                qWarning() << "[Step 4] Open DB failed:" << db.lastError().text();
                QSqlDatabase::removeDatabase(connName);
                continue;
            }

            QSqlQuery q(db);
            QString sql = "SELECT date, amount, category, outin FROM transactions";
            qDebug() << "[Step 4] Executing SQL:" << sql;
            if (!q.exec(sql)) {
                qWarning() << "[Step 4] Query failed:" << q.lastError().text();
                db.close();
                QSqlDatabase::removeDatabase(connName);
                continue;
            }

            while (q.next()) {
                QDate date = QDate::fromString(q.value(0).toString(), "yyyy-MM-dd");
                if (!date.isValid() || date < start || date > end) continue;

                double amount    = q.value(1).toDouble();
                QString category = q.value(2).toString();
                QString outin    = q.value(3).toString().trimmed();

                if (outin == QStringLiteral("收入")) {
                    sumIncome[category] += amount;
                }
                else if (outin == QStringLiteral("支出")) {
                    sumExpense[category] += amount;
                }
                else {
                    qWarning() << "[Step 4] Unknown outin value:" << outin;
                }
            }

            db.close();
        }
        QSqlDatabase::removeDatabase(connName);
    }

    qDebug() << "[Step 4] sumIncome =" << sumIncome;
    qDebug() << "[Step 4] sumExpense =" << sumExpense;

    QString baseName = QString("report_%1_%2")
                           .arg(start.toString("yyyyMMdd"))
                           .arg(end.toString("yyyyMMdd"));
    QString ext = fmt.toLower() == "csv" ? ".csv" : ".json";
    QString path = folder + "/" + baseName + ext;
    qDebug() << "[Step 5] Target file =" << path;

    if (QFile::exists(path)) {
        auto ret = QMessageBox::question(
            this, "文件已存在",
            QString("文件 \"%1\" 已存在，是否覆盖？").arg(baseName + ext),
            QMessageBox::Yes | QMessageBox::No
            );
        if (ret != QMessageBox::Yes) {
            qDebug() << "[Step 5] User cancelled overwrite";
            return;
        }
    }

    if (ext == ".csv") {
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "错误", "无法创建文件：" + path);
            return;
        }
        QTextStream out(&f);
        out << "类型,分类,金额\n";
        for (auto it = sumIncome.constBegin(); it != sumIncome.constEnd(); ++it)
            out << "收入," << it.key() << "," << it.value() << "\n";
        for (auto it = sumExpense.constBegin(); it != sumExpense.constEnd(); ++it)
            out << "支出," << it.key() << "," << it.value() << "\n";
        f.close();
        qDebug() << "[Step 6] CSV write completed";
    }
    else {
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, "错误", "无法创建文件：" + path);
            return;
        }
        QJsonObject root;
        QJsonArray arrInc, arrExp;
        for (auto it = sumIncome.constBegin(); it != sumIncome.constEnd(); ++it) {
            QJsonObject o; o["category"] = it.key(); o["amount"] = it.value();
            arrInc.append(o);
        }
        for (auto it = sumExpense.constBegin(); it != sumExpense.constEnd(); ++it) {
            QJsonObject o; o["category"] = it.key(); o["amount"] = it.value();
            arrExp.append(o);
        }
        root["income"]  = arrInc;
        root["expense"] = arrExp;
        f.write(QJsonDocument(root).toJson());
        f.close();
        qDebug() << "[Step 6] JSON write completed";
    }

    qDebug() << "=== Report Generation End ===";
    QMessageBox::information(this, "完成", QString("报表已生成：\n%1").arg(path));
}
