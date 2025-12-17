#ifndef PROJECTOVERVIEWTAB_H
#define PROJECTOVERVIEWTAB_H

#include <QWidget>
#include <QTextEdit>

class ProjectOverviewTab : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectOverviewTab(QWidget* parent = nullptr);

private:
    QTextEdit* content;
};

#endif // PROJECTOVERVIEWTAB_H
