#include "CategoryDelegate.h"
#include <QComboBox>
#include <QJsonObject>

CategoryDelegate::CategoryDelegate(QJsonArray categories, QObject *parent)
    : QStyledItemDelegate(parent), m_categories(categories) {}


QWidget* CategoryDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QComboBox *editor = new QComboBox(parent);
    for (const QJsonValue &v : m_categories) {
        QJsonObject obj = v.toObject();
        QString major = obj["name"].toString();
        editor->addItem(major);

        QJsonArray subs = obj["sub"].toArray();
        for (const QJsonValue &s : subs) {
            editor->addItem("    └ " + s.toString());
        }
    }
    return editor;
}



void CategoryDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QComboBox *combo = static_cast<QComboBox*>(editor);
    QString currentText = index.data(Qt::EditRole).toString();
    combo->setCurrentText(currentText);
}

void CategoryDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {
    QComboBox *combo = static_cast<QComboBox*>(editor);
    QString text = combo->currentText().trimmed();
    model->setData(index, text, Qt::EditRole);
}
