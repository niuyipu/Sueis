#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
    , transactionModel(nullptr)
{
    ui->setupUi(this);
    ensureUsersFolder();
;
    connect(ui->btnCreate, &QPushButton::clicked, this, [=](){ showHomePage(); ui->pages->setCurrentIndex(1); });
    connect(ui->btnCreate2, &QPushButton::clicked, this, [=](){ showHomePage(); ui->pages->setCurrentIndex(1); });
    connect(ui->btnDeleteUser, &QPushButton::clicked,this, &MainWindow::onDeleteUser);
    connect(ui->btnLogin,  &QPushButton::clicked, this, [=](){onLogin();});
    connect(ui->btnLogin2, &QPushButton::clicked, this,  [=](){onLogin();});

    connect(ui->btnConfirmCreate, &QPushButton::clicked, this,  [=](){onConfirmCreate(),onLogin();});

    connect(ui->btnSelectUser, &QPushButton::clicked, this, &MainWindow::onSelectUser);
    connect(ui->btnBrowseUsers, &QPushButton::clicked, this, &MainWindow::onBrowseUsers);

    connect(ui->btnConfirmLogin, &QPushButton::clicked, this, &MainWindow::onConfirmLogin);

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
    connect(ui->btnAddMajorCategory, &QPushButton::clicked,  this,  &MainWindow::AddMajorCategory);
    connect(ui->btnAddMinorCategory, &QPushButton::clicked,  this,  &MainWindow::AddMinorCategory);
    connect(ui->btnDeleteMajorCategory, &QPushButton::clicked,  this,  &MainWindow::DeleteMajorCategory);
    connect(ui->btnDeleteMinorCategory, &QPushButton::clicked,  this,  &MainWindow::DeleteMinorCategory);
    connect(ui->btnAnalysis, &QPushButton::clicked,this,&MainWindow::showAnalysisPage);
    connect(ui->btnApplyFilter, &QPushButton::clicked, this,&MainWindow::onApplyFilter);
    connect(this, &MainWindow::categoriesUpdated, this, &MainWindow::updateCategoryDelegate);
    connect(ui->btnSelectFolder, &QPushButton::clicked, this,&MainWindow::onSelectReportFolder);
    connect(ui->btnGenerateReport, &QPushButton::clicked, this,&MainWindow::onGenerateReport);

    transactionModel = new QSqlTableModel(this);
    transactionModel->setTable("transactions");
    transactionModel->select();

    initFilterControls();
    showHomePage();
}


MainWindow::~MainWindow() {
    delete ui;
}
