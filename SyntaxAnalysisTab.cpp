#include "SyntaxAnalysisTab.h"
#include <QFont>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidgetItem>
#include <QSet>
#include <QList>
#include <QChar>

SyntaxAnalysisTab::SyntaxAnalysisTab(QWidget* parent)
    : QWidget(parent)
{
    // ================= LEFT SIDE: Token Table =================
    tokenlabel = new QLabel("Token Table", this);
    tokenlabel->setFont(QFont("Poppins", 14, QFont::Bold));

    tokenizationtable = new QTableWidget(this);
    tokenizationtable->setColumnCount(4);
    tokenizationtable->setHorizontalHeaderLabels({"Token", "Type", "Line", "Column"});
    tokenizationtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokenizationtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tokenizationtable->verticalHeader()->setVisible(false);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(tokenlabel);
    leftLayout->addWidget(tokenizationtable);

    QWidget* leftContainer = new QWidget(this);
    leftContainer->setLayout(leftLayout);

    // ================= RIGHT SIDE: Inner Tabs =================
    innerTabWidget = new QTabWidget(this);

    tab1 = new QWidget();
    tab2 = new QWidget();
    tab3 = new QWidget();

    innerTabWidget->addTab(tab1, "Delimiter Parser");
    innerTabWidget->addTab(tab2, "Assignment Parser");
    innerTabWidget->addTab(tab3, "Operation Parser");

    innerTabWidget->setStyleSheet(R"(
        QTabBar::tab { padding: 11px 50px; font-family: 'Poppins'; font-size: 18px; font-weight: bold; }
        QTabBar::tab:selected { background: #16163F; color: white; }
        QTabBar::tab:!selected { background: rgba(160,160,160,0.3); color: rgba(160,160,160,0.6); }
    )");

    // ================= SETUP TABS =================
    auto setupValidatorTab = [](QWidget* tab, QTextEdit*& simulator, QTextEdit*& validator, QPushButton*& run) {
        QVBoxLayout* layout = new QVBoxLayout(tab);

        QLabel* simulatorTitle = new QLabel("Parser Log", tab);
        simulatorTitle->setFont(QFont("Poppins", 14, QFont::Bold));
        simulatorTitle->setAlignment(Qt::AlignCenter);
        layout->addWidget(simulatorTitle);

        simulator = new QTextEdit(tab);
        simulator->setReadOnly(true);
        simulator->setFont(QFont("Consolas", 12));
        layout->addWidget(simulator, 3);

        QLabel* validatorTitle = new QLabel("Validation Result", tab);
        validatorTitle->setFont(QFont("Poppins", 14, QFont::Bold));
        validatorTitle->setAlignment(Qt::AlignCenter);
        layout->addWidget(validatorTitle);

        validator = new QTextEdit(tab);
        validator->setReadOnly(true);
        validator->setFont(QFont("Poppins", 12));
        layout->addWidget(validator, 1);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        run = new QPushButton("Run Parser", tab);
        run->setFont(QFont("Poppins", 10, QFont::Bold));
        run->setStyleSheet("background-color: #16163F; color: white; padding: 10px 30px;");
        buttonLayout->addStretch();
        buttonLayout->addWidget(run);
        layout->addLayout(buttonLayout);
        tab->setLayout(layout);
    };

    setupValidatorTab(tab1, tab1Simulator, tab1Validator, tab1Run);
    setupValidatorTab(tab2, tab2Simulator, tab2Validator, tab2Run);
    setupValidatorTab(tab3, tab3Simulator, tab3Validator, tab3Run);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(leftContainer, 2);
    mainLayout->addWidget(innerTabWidget, 3);
    setLayout(mainLayout);

    // ================= HELPER: Extract Tokens =================
    auto getTokensWithLines = [this]() -> QList<QPair<QString, int>> {
        QList<QPair<QString, int>> tokens;
        for (int i = 0; i < tokenizationtable->rowCount(); ++i) {
            QTableWidgetItem* tokenItem = tokenizationtable->item(i, 0);
            QTableWidgetItem* lineItem = tokenizationtable->item(i, 2);
            if (tokenItem && !tokenItem->text().trimmed().isEmpty()) {
                QString token = tokenItem->text();
                int line = lineItem ? lineItem->text().toInt() : (i + 1);
                tokens.append(qMakePair(token, line));
            }
        }
        return tokens;
    };

    // ================= DELIMITER PARSER (PDA) =================
    connect(tab1Run, &QPushButton::clicked, this, [this, getTokensWithLines]() {
        auto tokens = getTokensWithLines();
        tab1Simulator->clear();
        tab1Validator->clear();

        QStringList log;
        log << "Context-Free Grammar (CFG):\n";
        log << "S → S S\n";
        log << "S → { S }\n";
        log << "S → ( S )\n";
        log << "S → [ S ]\n";
        log << "S → ε\n\n";
        log << "PDA Simulation: Delimiter Matching\n\n";

        QSet<QChar> delimSet = {'{', '}', '(', ')', '[', ']'};
        QList<QPair<QChar, int>> delimTokens;

        for (const auto& pair : tokens) {
            if (pair.first.length() == 1 && delimSet.contains(pair.first[0])) {
                delimTokens.append(qMakePair(pair.first[0], pair.second));
            }
        }

        if (delimTokens.isEmpty()) {
            log << "No delimiter tokens found.\n";
            tab1Simulator->setPlainText(log.join(""));
            tab1Validator->setPlainText("✅ No delimiters to validate");
            return;
        }

        QStringList stack;
        bool valid = true;
        int step = 1;

        for (const auto& dpair : delimTokens) {
            QChar ch = dpair.first;
            int line = dpair.second;

            if (ch == '{' || ch == '(' || ch == '[') {
                stack.push_back(QString(ch));
                log << QString("Step %1: READ '%2' (line %3) → PUSH '%2'\n")
                           .arg(step++).arg(ch).arg(line);
            } else {
                if (stack.isEmpty()) {
                    log << QString("Step %1: ❌ ERROR closing '%2' on empty stack (line %3)\n")
                           .arg(step++).arg(ch).arg(line);
                    valid = false;
                    break;
                }

                QChar top = stack.last()[0];
                bool match = (ch == '}' && top == '{') ||
                             (ch == ')' && top == '(') ||
                             (ch == ']' && top == '[');

                if (match) {
                    stack.pop_back();
                    log << QString("Step %1: READ '%2' (line %3) → POP '%4' (matched)\n")
                           .arg(step++).arg(ch).arg(line).arg(top);
                } else {
                    log << QString("Step %1: ❌ MISMATCH '%2' vs '%3'\n")
                               .arg(step++).arg(top).arg(ch);
                    valid = false;
                    break;
                }
            }
        }

        if (valid && !stack.isEmpty()) {
            log << "\n❌ ERROR: Unclosed delimiters remain\n";
            valid = false;
        }

        tab1Simulator->setPlainText(log.join(""));
        tab1Validator->setPlainText(valid ? "✅ Delimiters are balanced" : "❌ Delimiters are unbalanced");
    });

// ================= ASSIGNMENT PARSER (PDA) =================
connect(tab2Run, &QPushButton::clicked, this, [this, getTokensWithLines]() {
    auto tokens = getTokensWithLines();
    tab2Simulator->clear();
    tab2Validator->clear();

    QStringList log;
    log << "Context-Free Grammar (CFG):\n";
    log << "S → id = E\n";
    log << "E → E + T | E - T | T\n";
    log << "T → T * F | T / F | F\n";
    log << "F → ( E ) | id | number\n\n";
    log << "PDA Simulation: Assignment Parsing\n\n";

    if (tokens.isEmpty()) {
        log << "No tokens available.\n";
        tab2Simulator->setPlainText(log.join(""));
        tab2Validator->setText("❌ No assignments to parse");
        return;
    }

    int step = 1;
    int currentLine = -1;
    QString currentId;
    QStringList stack;
    int successCount = 0;
    int failCount = 0;

    auto isValidId = [](const QString& token){
        if (token.isEmpty()) return false;
        QChar first = token[0];
        return first.isLetter() || first == '_';
    };

    // Define keywords to skip
    QSet<QString> keywords = {"if", "else", "print", "while", "for"}; // add more as needed

    for (const auto& pair : tokens) {
        QString token = pair.first;
        int line = pair.second;

        if (line != currentLine) {
            if (currentLine != -1) {
                if (!stack.isEmpty()) {
                    log << "\n❌ ERROR: Unclosed parentheses remain\n";
                    failCount++;
                } else if (!currentId.isEmpty()) {
                    successCount++;
                }
            }

            stack.clear();
            currentId.clear();
            currentLine = line;
            log << QString("\nAssignment Statement (line %1):\n").arg(line);

            // Skip lines starting with keywords
            if (keywords.contains(token)) {
                log << QString("Skipped line %1: starts with keyword '%2'\n").arg(line).arg(token);
                continue;
            }
        }

        bool valid = true;

        if (currentId.isEmpty()) {
            if (isValidId(token)) {
                currentId = token;
                log << QString("Step %1: READ '%2' (line %3) → matches 'id'\n").arg(step++).arg(token).arg(line);
            } else {
                log << QString("Step %1: ❌ ERROR, expected 'id' but got '%2'\n").arg(step++).arg(token);
                valid = false;
            }
        } else if (stack.isEmpty() && token == "=") {
            log << QString("Step %1: READ '%2' (line %3) → matches '='\n").arg(step++).arg(token).arg(line);
        } else {
            if (token == "(") {
                stack.push_back("(");
                log << QString("Step %1: READ '%2' (line %3) → PUSH '('\n").arg(step++).arg(token).arg(line);
            } else if (token == ")") {
                if (stack.isEmpty()) {
                    log << QString("Step %1: ❌ ERROR, unexpected ')'\n").arg(step++);
                    valid = false;
                } else {
                    stack.pop_back();
                    log << QString("Step %1: READ '%2' (line %3) → POP '(' (matched)\n").arg(step++).arg(token).arg(line);
                }
            } else if (token[0].isDigit()) {
                log << QString("Step %1: READ '%2' (line %3) → matches 'number'\n").arg(step++).arg(token).arg(line);
            } else if (token == "+" || token == "-" || token == "*" || token == "/") {
                log << QString("Step %1: READ '%2' (line %3) → matches operator\n").arg(step++).arg(token).arg(line);
            } else if (isValidId(token)) {
                log << QString("Step %1: READ '%2' (line %3) → matches 'id'\n").arg(step++).arg(token).arg(line);
            } else {
                log << QString("Step %1: ❌ ERROR, unexpected token '%2'\n").arg(step++).arg(token);
                valid = false;
            }
        }

        if (!valid) failCount++;
    }

    if (!stack.isEmpty()) {
        log << "\n❌ ERROR: Unclosed parentheses remain\n";
        failCount++;
    } else if (!currentId.isEmpty()) {
        successCount++;
    }

    tab2Simulator->setPlainText(log.join(""));
    tab2Validator->setText(QString("✅ Success: %1 statements\n❌ Failures: %2 statements")
                           .arg(successCount).arg(failCount));
});


    // ================= OPERATION PARSER (PDA) =================
    connect(tab3Run, &QPushButton::clicked, this, [this, getTokensWithLines]() {
        auto tokens = getTokensWithLines();
        tab3Simulator->clear();
        tab3Validator->clear();

        QStringList log;
        log << "Context-Free Grammar (CFG) for Operations:\n";
        log << "E → E + T | E - T | T\n";
        log << "T → T * F | T / F | F\n";
        log << "F → ( E ) | id | number\n\n";
        log << "PDA Simulation: Operation Parsing\n\n";

        int statementCounter = 1;
        int successCount = 0;
        int failCount = 0;
        QList<QPair<QString,int>> currentExpr;

        auto isExprToken = [](const QString& token){
            return !token.isEmpty() && (token[0].isLetter() || token[0].isDigit() ||
                   token == "+" || token == "-" || token == "*" || token == "/" ||
                   token == "(" || token == ")");
        };

        for (int i = 0; i < tokens.size(); ++i) {
            QString tok = tokens[i].first;
            int line = tokens[i].second;

            if (isExprToken(tok)) currentExpr.append(tokens[i]);

            bool isEndOfExpr = (i+1 == tokens.size()) || !isExprToken(tokens[i+1].first);

            if (!currentExpr.isEmpty() && isEndOfExpr) {
                log << QString("Expression %1:\n").arg(statementCounter++);
                int step = 1;
                QStringList stack;
                bool valid = true;

                for (const auto& pair : currentExpr) {
                    QString token = pair.first;
                    int line = pair.second;

                    if (token == "(") {
                        stack.push_back(token);
                        log << QString("Step %1: READ '(' (line %2) → PUSH '('\n").arg(step++).arg(line);
                    } else if (token == ")") {
                        if (stack.isEmpty() || stack.last() != "(") {
                            log << QString("Step %1: ❌ ERROR, unexpected ')'\n").arg(step++);
                            valid = false;
                            break;
                        } else {
                            stack.pop_back();
                            log << QString("Step %1: READ ')' (line %2) → POP '(' (matched)\n").arg(step++).arg(line);
                        }
                    } else if (token[0].isLetter()) {
                        log << QString("Step %1: READ '%2' (line %3) → matches 'id'\n").arg(step++).arg(token).arg(line);
                    } else if (token[0].isDigit()) {
                        log << QString("Step %1: READ '%2' (line %3) → matches 'number'\n").arg(step++).arg(token).arg(line);
                    } else if (token == "+" || token == "-" || token == "*" || token == "/") {
                        log << QString("Step %1: READ '%2' (line %3) → matches operator\n").arg(step++).arg(token).arg(line);
                    }
                }

                if (valid && !stack.isEmpty()) {
                    log << "\n❌ ERROR: Unclosed parentheses remain\n";
                    valid = false;
                }

                log << (valid ? "✅ Expression parsed successfully!\n\n" : "❌ Expression parsing failed.\n\n");

                currentExpr.clear();
                if (valid) successCount++;
                else failCount++;
            }
        }

        tab3Simulator->setPlainText(log.join(""));
        tab3Validator->setText(QString("✅ Success: %1 expressions\n❌ Failures: %2 expressions")
                               .arg(successCount).arg(failCount));
    });
}

// ================= UPDATE TOKEN TABLE =================
void SyntaxAnalysisTab::updateTokenTable(const QList<QList<QString>>& tokens)
{
    tokenizationtable->setRowCount(tokens.size());
    for (int i = 0; i < tokens.size(); ++i) {
        for (int j = 0; j < qMin(tokens[i].size(), 4); ++j) {
            tokenizationtable->setItem(i, j, new QTableWidgetItem(tokens[i][j]));
        }
    }
}
