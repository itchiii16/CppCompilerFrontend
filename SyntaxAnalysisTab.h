#ifndef SYNTAXANALYSISTAB_H
#define SYNTAXANALYSISTAB_H

#include <QWidget>
#include <QList>
#include <QPair>
#include <QSet>

class QLabel;
class QTableWidget;
class QTabWidget;
class QTextEdit;
class QPushButton;
class QGraphicsView;
class QGraphicsScene;

class SyntaxAnalysisTab : public QWidget
{
    Q_OBJECT

public:
    SyntaxAnalysisTab(QWidget* parent = nullptr);
    void updateTokenTable(const QList<QList<QString>>& tokens);

private:
    // Token Table
    QLabel* tokenlabel;
    QTableWidget* tokenizationtable;

    // Inner Tabs
    QTabWidget* innerTabWidget;

    // Delimiter Validator
    QWidget* tab1;
    QTextEdit* tab1Simulator;
    QTextEdit* tab1Validator;
    QPushButton* tab1Run;

    // Assignment Validator
    QWidget* tab2;
    QTextEdit* tab2Simulator;
    QTextEdit* tab2Validator;
    QPushButton* tab2Run;

    // Operation Validator
    QWidget* tab3;
    QTextEdit* tab3Simulator;
    QTextEdit* tab3Validator;
    QPushButton* tab3Run;

    // 🔹 NFA Simulator (Graphical)
    QWidget* nfaTab;
    QGraphicsView* nfaGraphicsView;
    QPushButton* nfaRun;
};

#endif // SYNTAXANALYSISTAB_H