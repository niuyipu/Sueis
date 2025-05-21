#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QMessageBox>
#include <QtSql>
#include <QFileDialog>
#include <QRegularExpression>

#include "xlsxdocument.h"
using namespace QXlsx;

static QDate excelBaseDate() { return QDate(1899, 12, 30); }

static const QRegularExpression dateRe(QStringLiteral("(日|日期|时|Date)"), QRegularExpression::CaseInsensitiveOption);

void MainWindow::onImportBill() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "选择账单文件", QString(),
        "Excel 文件 (*.xlsx *.xls *.csv)"
        );
    if (fileName.isEmpty()) return;

    ui->labelSelectedFile->setText(fileName);
    ui->pages->setCurrentIndex(6);

    Document xlsx(fileName);
    int rowCount = xlsx.dimension().rowCount();
    int colCount = xlsx.dimension().columnCount();
    if (rowCount < 2 || colCount == 0) {
        QMessageBox::warning(
            this, "错误",
            "无法读取 Excel 文件，可能文件格式有问题！"
            );
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
            QString text;
            if (cell) {
                QVariant v = cell->value();
                if (v.type() == QVariant::Double
                    && dateRe.match(headers[idx]).hasMatch())
                {
                    double serial = v.toDouble();
                    QDate dt = excelBaseDate().addDays(int(serial));
                    text = dt.isValid()
                               ? dt.toString("yyyy-MM-dd") : QString();
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
        auto btn = QMessageBox::question(
            this, tr("无可用账户"),
            tr("当前没有可选账户，是否要先创建一个账户？"),
            QMessageBox::Yes | QMessageBox::No
            );
        if (btn == QMessageBox::Yes) {
            showDashboard();
            onAddAccount();
        } else {
            showDashboard();
        }
        return;
    }

    QString target = ui->comboTargetAccount->currentText();
    QString dbPath = QDir::currentPath()
                     + "/users/" + currentUser + "/" + target + ".db";
    QSqlDatabase db = QSqlDatabase::addDatabase(
        "QSQLITE", target + "_import"
        );
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        QMessageBox::critical(
            this, "错误", "无法打开目标数据库！"
            );
        return;
    }

    QSqlTableModel model(this, db);
    model.setTable("transactions");
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.select();

    int rowCount = previewModel->rowCount();
    int colCount = previewModel->columnCount();

    int dateCol = -1;
    for (int i = 0; i < colCount; ++i) {
        QString hdr = previewModel->headerData(i, Qt::Horizontal)
        .toString();
        if (dateRe.match(hdr).hasMatch()) {
            dateCol = i;
            break;
        }
    }
    for (int r = 0; r < rowCount; ++r) {
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
            QString txt    = previewModel->item(r, c)
                              ->text().trimmed();

            if (c == dateCol) {
                QDate dt = QDate::fromString(txt, "yyyy-MM-dd");
                if (!dt.isValid() && !txt.isEmpty()) {
                    double serial = txt.toDouble();
                    dt = excelBaseDate().addDays(int(serial));
                }
                rec.setValue("date", dt);
            }
            else if (QRegularExpression("商品", QRegularExpression::CaseInsensitiveOption)
                         .match(header).hasMatch()) {
                rec.setValue("note", txt);
            }
            else if (QRegularExpression("(收|支|In|Out)",
                                        QRegularExpression::CaseInsensitiveOption)
                         .match(header).hasMatch()) {
                rec.setValue("outin", txt);
            }
            else if (QRegularExpression("金",
                                        QRegularExpression::CaseInsensitiveOption)
                         .match(header).hasMatch()) {
                rec.setValue("amount", txt.toDouble());
            }
        }
        model.insertRecord(-1, rec);
    }

    if (!model.submitAll()) {
        QMessageBox::warning(
            this, "导入失败", model.lastError().text()
            );
    } else {
        updateCurrentBalance();
        auto matches = ui->listAccounts->findItems(
            target, Qt::MatchExactly
            );
        if (!matches.isEmpty())
            ui->listAccounts->setCurrentItem(matches.first());
        onEditAccount();
    }

    QSqlDatabase::removeDatabase(target + "_import");
}
