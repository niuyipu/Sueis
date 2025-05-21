#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDir>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QDesktopServices>

void MainWindow::ensureUsersFolder() {
    QDir dir(QDir::currentPath() + "/users");
    if (!dir.exists()) dir.mkpath(dir.path());
}

void MainWindow::showHomePage() {
    ui->pages->setCurrentIndex(0);
}

void MainWindow::showDashboard() {
    QDate earliest = QDate::currentDate();
    QDate latest = QDate::currentDate();

    ui->dateEditStart->setDate(earliest);
    ui->dateEditEnd->setDate(latest);

    ui->dateStart->setDate(earliest);
    ui->dateEnd->setDate(latest);
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

void MainWindow::createUser(const QString &username, const QString &password) {
    QString path = QDir::currentPath() + "/users/" + username;
    if (QDir(path).exists()) { QMessageBox::warning(this, "错误", "用户已存在");
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

    const QString defaultJsonPath = "default.json";
    QFile src(defaultJsonPath);
    if (!src.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开 default.json：" << src.errorString();
        return;
    }
    QByteArray data = src.readAll();
    src.close();

    QJsonParseError err;
    QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "default.json 格式不合法：" << err.errorString();
    }

    const QString destPath = path + "/category.json";
    QFile dst(destPath);
    if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "无法写入" << destPath << "：" << dst.errorString();
        return;
    }
    dst.write(data);
    dst.close();

    qDebug() << "已将" << defaultJsonPath << "转写到" << destPath;

    QMessageBox::information(this, "成功", "用户创建成功");
}

void MainWindow::onDeleteUser()
{
    auto item = ui->listExistingUsers->currentItem();
    if (!item) {
        QMessageBox::warning(this, "错误", "当前没有选中的用户");
        return;
    }
    selectedUser = item->text();
    QString username = selectedUser;

    auto ret = QMessageBox::question(
        this, "删除确认",
        QString("确定要删除用户 \"%1\" 吗？此操作会删除所有数据且不可恢复。").arg(username),
        QMessageBox::Yes | QMessageBox::No
        );
    if (ret != QMessageBox::Yes)
        return;

    QString userDirPath = QDir::currentPath() + "/users/" + username;
    QDir userDir(userDirPath);
    if (!userDir.exists()) {
        QMessageBox::warning(this, "错误", "用户目录不存在");
        return;
    }

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

void MainWindow::onBrowseUsers() {
    QString usersPath = QDir::currentPath() + "/users";
    QDesktopServices::openUrl(QUrl::fromLocalFile(usersPath));
}

bool MainWindow::verifyPassword(const QString &username, const QString &password) {
    QString cfg = QDir::currentPath() + "/users/" + username + "/config.json";
    QFile f(cfg); if (!f.open(QIODevice::ReadOnly)) return false;
    QString stored = QJsonDocument::fromJson(f.readAll()).object()["password"].toString();
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return stored == QString(hash.toHex());
}

void MainWindow::onConfirmLogin() {
    QString pwd = ui->editLoginPassword->text();
    if (!verifyPassword(selectedUser, pwd)) {
        QMessageBox::warning(this, "错误", "用户名或密码错误");
        return;
    }
    enterDashboard(selectedUser);
}

void MainWindow::enterDashboard(const QString &username) {
    isLoggedIn = true;
    currentUser = username;
    ui->labelWelcome->setText(QString("欢迎回来, %1").arg(username));
    loadAccounts(username);
    showDashboard();
    updateTotalBalance();
}

void MainWindow::onLogout() {
    isLoggedIn = false;
    ui->editLoginPassword->clear();
    showHomePage();
}
