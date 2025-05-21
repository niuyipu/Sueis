#include "mainwindow.h"
#include "ui_MainWindow.h"
#include <QDir>
#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>


void MainWindow::showeditcategory(){
    ui->pages->setCurrentIndex(7);
    loadCategories(selectedUser);
}
void MainWindow::loadCategories(const QString &username) {
    QString path = QDir::currentPath() + "/users/" + username + "/category.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        ui->treeCategories->clear();
        return;
    }

    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        ui->treeCategories->clear();
        return;
    }
    QJsonArray arr = doc.object().value("categories").toArray();

    ui->treeCategories->clear();
    ui->treeCategories->setHeaderLabel(tr("类别"));

    for (const QJsonValue &v : arr) {
        if (!v.isObject()) continue;
        QJsonObject obj = v.toObject();

        QString majorName = obj.value("name").toString();
        QTreeWidgetItem *majorItem = new QTreeWidgetItem(ui->treeCategories);
        majorItem->setText(0, majorName);

        QJsonArray subs = obj.value("sub").toArray();
        for (const QJsonValue &s : subs) {
            if (s.isString()) {
                QTreeWidgetItem *subItem = new QTreeWidgetItem(majorItem);
                subItem->setText(0, s.toString());
            }
        }
        majorItem->setExpanded(true);
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
void MainWindow::AddMajorCategory() {
    bool ok = false;
    QString name = QInputDialog::getText(
        this,
        tr("添加大类"),
        tr("请输入大类名称："),
        QLineEdit::Normal,
        QString(),
        &ok
        );
    if (!ok) return;
    name = name.trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("名称不能为空"));
        return;
    }

    QRegularExpression regex(QStringLiteral("^[\u4e00-\u9fa5a-zA-Z0-9]+$"));
    if (!regex.match(name).hasMatch()) {
        QMessageBox::warning(this, tr("无效名称"), tr("名称只能包含中文、字母和数字"));
        return;
    }

    QString path = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile file(path);
    QJsonObject rootObj;
    QJsonArray catArr;
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject())
            catArr = doc.object().value("categories").toArray();
        file.close();
    } else {
        QDir().mkpath(QFileInfo(path).absolutePath());
    }

    for (const QJsonValue &v : catArr) {
        if (v.isObject() && v.toObject().value("name").toString() == name) {
            QMessageBox::information(this, tr("提示"), tr("该大类已存在"));
            return;
        }
    }

    QJsonObject newMajor;
    newMajor.insert("name", name);
    newMajor.insert("sub", QJsonArray());
    catArr.append(newMajor);

    rootObj.insert("categories", catArr);
    QJsonDocument outDoc(rootObj);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("错误"), tr("无法写入类别文件：%1").arg(file.errorString()));
        return;
    }
    file.write(outDoc.toJson(QJsonDocument::Indented));
    file.close();

    loadCategories(currentUser);
    emit categoriesUpdated();
}



void MainWindow::AddMinorCategory() {
    QStringList majors;
    QString path = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile file(path);
    QJsonArray catArr;
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject())
            catArr = doc.object().value("categories").toArray();
        file.close();
    }
    for (const QJsonValue &v : catArr) {
        if (v.isObject()) {
            majors.append(v.toObject().value("name").toString());
        }
    }

    bool okMajor = false;
    QString selectedMajor = QInputDialog::getItem(
        this,tr("选择大类"),tr("请选择要添加小类的所属大类："),majors,0,false,&okMajor);
    if (!okMajor) return;

    bool okSub = false;
    QString subName = QInputDialog::getText( this,tr("添加小类"),tr("请输入小类名称："),QLineEdit::Normal, QString(),&okSub);
    if (!okSub) return;
    subName = subName.trimmed();
    if (subName.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("名称不能为空"));
        return;
    }

    QRegularExpression regex(QStringLiteral("^[\u4e00-\u9fa5a-zA-Z0-9]+$"));
    if (!regex.match(subName).hasMatch()) {
        QMessageBox::warning(this, tr("无效名称"), tr("名称只能包含中文、字母和数字"));
        return;
    }

    QJsonObject rootObj;
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject())
            rootObj = doc.object();
        file.close();
    }
    catArr = rootObj.value("categories").toArray();

    for (int i = 0; i < catArr.size(); ++i) {
        QJsonObject obj = catArr[i].toObject();
        if (obj.value("name").toString() == selectedMajor) {
            QJsonArray subs = obj.value("sub").toArray();
            // 去重
            for (const QJsonValue &s : subs) {
                if (s.toString() == subName) {
                    QMessageBox::information(this, tr("提示"), tr("该小类已存在"));
                    return;
                }
            }
            subs.append(subName);
            obj.insert("sub", subs);
            catArr[i] = obj;
            break;
        }
    }
    rootObj.insert("categories", catArr);
    QJsonDocument outDoc(rootObj);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("错误"), tr("无法写入类别文件：%1").arg(file.errorString()));
        return;
    }
    file.write(outDoc.toJson(QJsonDocument::Indented));
    file.close();
    loadCategories(currentUser);
    emit categoriesUpdated();
}

void MainWindow::DeleteMajorCategory() {
    QString path = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("提示"), tr("没有可删除的大类"));
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        QMessageBox::warning(this, tr("错误"), tr("类别文件格式不正确"));
        return;
    }
    QJsonArray catArr = doc.object().value("categories").toArray();
    if (catArr.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("没有可删除的大类"));
        return;
    }

    QStringList majors;
    for (const QJsonValue &v : catArr) {
        QJsonObject obj = v.toObject();
        majors.append(obj.value("name").toString());
    }
    bool ok = false;
    QString toDelete = QInputDialog::getItem(
        this, tr("删除大类"), tr("请选择要删除的大类："),
        majors, 0, false, &ok
        );
    if (!ok) return;

    if (QMessageBox::question(
            this, tr("确认删除"),
            tr("确定要删除大类“%1”及其所有子类吗？").arg(toDelete),
            QMessageBox::Yes|QMessageBox::No, QMessageBox::No
            ) != QMessageBox::Yes) {
        return;
    }

    for (int i = 0; i < catArr.size(); ++i) {
        QJsonObject obj = catArr[i].toObject();
        if (obj.value("name").toString() == toDelete) {
            catArr.removeAt(i);
            break;
        }
    }

    QJsonObject rootObj;
    rootObj.insert("categories", catArr);
    QJsonDocument outDoc(rootObj);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无法写入类别文件：%1").arg(file.errorString()));
        return;
    }
    file.write(outDoc.toJson(QJsonDocument::Indented));
    file.close();

    loadCategories(currentUser);
}


void MainWindow::DeleteMinorCategory() {
    QString path = QDir::currentPath() + "/users/" + currentUser + "/category.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("提示"), tr("没有可删除的小类"));
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        QMessageBox::warning(this, tr("错误"), tr("类别文件格式不正确"));
        return;
    }
    QJsonArray catArr = doc.object().value("categories").toArray();

    QStringList majors;
    for (const QJsonValue &v : catArr) {
        QJsonObject obj = v.toObject();
        majors.append(obj.value("name").toString());
    }
    bool okMajor = false;
    QString major = QInputDialog::getItem(
        this, tr("选择大类"), tr("请选择所属大类："),
        majors, 0, false, &okMajor
        );
    if (!okMajor) return;
    int majorIndex = -1;
    QStringList subs;
    for (int i = 0; i < catArr.size(); ++i) {
        QJsonObject obj = catArr[i].toObject();
        if (obj.value("name").toString() == major) {
            majorIndex = i;
            for (const QJsonValue &sv : obj.value("sub").toArray())
                subs.append(sv.toString());
            break;
        }
    }
    if (majorIndex < 0 || subs.isEmpty()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("大类“%1”下没有可删除的小类").arg(major));
        return;
    }
    bool okSub = false;
    QString sub = QInputDialog::getItem(
        this, tr("删除小类"), tr("请选择要删除的小类："),
        subs, 0, false, &okSub
        );
    if (!okSub) return;
    if (QMessageBox::question(
            this, tr("确认删除"),
            tr("确定要删除“%1”下的小类“%2”吗？").arg(major, sub),
            QMessageBox::Yes|QMessageBox::No, QMessageBox::No
            ) != QMessageBox::Yes) {
        return;}
    QJsonObject majorObj = catArr[majorIndex].toObject();
    QJsonArray subArr = majorObj.value("sub").toArray();
    for (int j = 0; j < subArr.size(); ++j) {
        if (subArr[j].toString() == sub) {
            subArr.removeAt(j);
            break;}
    }
    majorObj.insert("sub", subArr);
    catArr[majorIndex] = majorObj;

    QJsonObject rootObj;
    rootObj.insert("categories", catArr);
    QJsonDocument outDoc(rootObj);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无法写入类别文件：%1").arg(file.errorString()));
        return;
    }
    file.write(outDoc.toJson(QJsonDocument::Indented));
    file.close();
    loadCategories(currentUser);
}
