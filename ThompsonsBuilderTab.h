#ifndef THOMPSONSBUILDER_TAB_H
#define THOMPSONSBUILDER_TAB_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QGraphicsView>
#include <QPushButton>

class ThompsonsBuilderTab : public QWidget
{
    Q_OBJECT

public:
    explicit ThompsonsBuilderTab(QWidget *parent = nullptr);

private slots:
    void buildNFA();

private:
    struct NFAState {
        int id;
        bool isAccept = false;
    };

    struct NFATransition {
        int from;
        QString symbol; // "ε" or character
        int to;
    };

    struct BuildStep {
        QString description;
    };

    QLineEdit* regexInput;
    QTextEdit* buildLog;
    QTextEdit* stepLog;
    QGraphicsView* graphicsView;
    QPushButton* buildButton;

    using NFA = struct {
        int startState;
        int acceptState;
        QList<NFAState> states;
        QList<NFATransition> transitions;
    };

    QList<BuildStep> buildSteps; // Tracks construction steps

    NFA createSymbolNFA(const QString& symbol);
    NFA unionNFA(const NFA& n1, const NFA& n2);
    NFA concatNFA(const NFA& n1, const NFA& n2);
    NFA closureNFA(const NFA& n);
    NFA buildFromRegex(const QString& regex);
    void drawNFA(const NFA& nfa);
};

#endif // THOMPSONSBUILDER_TAB_H