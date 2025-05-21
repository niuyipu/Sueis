#pragma once
#include <QStyledItemDelegate>
#include <QJsonArray>

class CategoryDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit CategoryDelegate(QJsonArray categories, QObject *parent = nullptr);
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
private:
    QJsonArray m_categories;
};
