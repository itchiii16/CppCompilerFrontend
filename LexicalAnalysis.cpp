#include "LexicalAnalysis.h"
#include <QFont>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QRegularExpression>
#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBrush>
#include <QPen>
#include <QPainterPath>
#include <QMap>
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QRectF>
#include <QLineF>
#include <QPolygonF>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>

// ==========================
//   Helper Functions
// ==========================

// A function that builds a simple symbol NFA for a character
NFA buildSymbolNFA(QChar c)
{
    NFA nfa;
    int s0 = nfa.getNextId();
    int s1 = nfa.getNextId();
    nfa.states.append({s0, false});
    nfa.states.append({s1, true});
    nfa.transitions.append({s0, QString(c), s1});
    nfa.startState = s0;
    nfa.acceptState = s1;
    return nfa;
}

// Closure operation: nfa* (Kleene Star)
NFA buildClosureNFA(const NFA& n)
{
    NFA result;
    int nextId = 0;

    result.states.append({nextId++, false});
    result.states.append({nextId++, true});
    int newStart = result.states[0].id;
    int newAccept = result.states[1].id;

    QMap<int, int> idMap;
    for (const auto& s : n.states) {
        idMap[s.id] = nextId++;
        result.states.append({idMap[s.id], s.isAccept});
    }

    result.transitions.append({newStart, "ε", idMap[n.startState]});
    result.transitions.append({newStart, "ε", newAccept});

    for (const auto& s : n.states) {
        if (s.isAccept) {
            result.transitions.append({idMap[s.id], "ε", idMap[n.startState]});
            result.transitions.append({idMap[s.id], "ε", newAccept});
            for (auto& rs : result.states) {
                if (rs.id == idMap[s.id]) rs.isAccept = false;
            }
        }
    }

    for (const auto& t : n.transitions) {
        result.transitions.append({idMap[t.from], t.symbol, idMap[t.to]});
    }

    result.startState = newStart;
    result.acceptState = newAccept;
    return result;
}

// Concatenation operation: nfa1 + nfa2
NFA buildConcatNFA(const NFA& n1, const NFA& n2)
{
    NFA result = n1;
    int offset = n1.states.size();

    for (const auto& s : n2.states) {
        result.states.append({s.id + offset, s.isAccept});
    }

    for (auto& s : result.states) {
        if (s.id == n1.acceptState) {
            s.isAccept = false;
        }
    }

    result.transitions.append({n1.acceptState, "ε", n2.startState + offset});

    for (const auto& t : n2.transitions) {
        result.transitions.append({t.from + offset, t.symbol, t.to + offset});
    }

    result.acceptState = n2.acceptState + offset;
    return result;
}

// Union operation: nfa1 | nfa2
NFA buildUnionNFA(const NFA& n1, const NFA& n2)
{
    NFA result;
    int nextId = 0;

    result.states.append({nextId++, false});
    result.states.append({nextId++, true});
    int newStart = result.states[0].id;
    int newAccept = result.states[1].id;

    QMap<int, int> idMap;
    auto addStates = [&](const QList<NFAState>& states) {
        for (const auto& s : states) {
            idMap[s.id] = nextId++;
            result.states.append({idMap[s.id], s.isAccept});
        }
    };

    addStates(n1.states);
    addStates(n2.states);

    result.transitions.append({newStart, "ε", idMap[n1.startState]});
    result.transitions.append({newStart, "ε", idMap[n2.startState]});

    for (const auto& s : n1.states) {
        if (s.isAccept) {
            result.transitions.append({idMap[s.id], "ε", newAccept});
        }
    }
    for (const auto& s : n2.states) {
        if (s.isAccept) {
            result.transitions.append({idMap[s.id], "ε", newAccept});
        }
    }

    for (const auto& t : n1.transitions) {
        result.transitions.append({idMap[t.from], t.symbol, idMap[t.to]});
    }
    for (const auto& t : n2.transitions) {
        result.transitions.append({idMap[t.from], t.symbol, idMap[t.to]});
    }

    result.startState = newStart;
    result.acceptState = newAccept;
    return result;
}

// ==========================
//   Token-Specific NFA Builder
// ==========================

// Identifier NFA: Matches [a-zA-Z_][a-zA-Z0-9_]*
NFA buildIdentifierNFA()
{
    NFA first = buildSymbolNFA('a');  // First character: [a-zA-Z_]
    NFA loop = buildClosureNFA(first); // Loop for subsequent characters: [a-zA-Z0-9_]
    return loop;
}

// Number NFA: Matches digits and optional decimal point (e.g., 123, 123.45)
NFA buildNumberNFA()
{
    NFA digit = buildSymbolNFA('0'); // First digit [0-9]
    NFA dot = buildSymbolNFA('.');   // Decimal point
    NFA decimal = buildConcatNFA(dot, digit); // Handle decimals like 12.34
    return buildUnionNFA(digit, decimal);    // Handle integer or decimal numbers
}

// String Literal NFA: Matches specific string literals (e.g., "Hello")
NFA buildStringLiteralNFA(const QString& str)
{
    NFA nfa;
    int lastState = nfa.getNextId();
    nfa.states.append({lastState, false});
    for (int i = 0; i < str.length(); ++i) {
        int currentState = nfa.getNextId();
        nfa.states.append({currentState, false});
        nfa.transitions.append({lastState, QString(str[i]), currentState});
        lastState = currentState;
    }
    int acceptState = nfa.getNextId();
    nfa.states.append({acceptState, true});
    nfa.transitions.append({lastState, "ε", acceptState});
    nfa.startState = nfa.states[0].id;
    nfa.acceptState = acceptState;
    return nfa;
}

// ==========================
//   DrawHelper IMPLEMENTATION
// ==========================

QGraphicsEllipseItem* DrawHelper::createState(QGraphicsScene* scene, qreal x, qreal y, qreal size)
{
    auto* state = scene->addEllipse(0, 0, size, size, QPen(Qt::black), QBrush(Qt::white));
    state->setPos(x, y);
    state->setFlag(QGraphicsItem::ItemIsMovable);
    state->setFlag(QGraphicsItem::ItemIsSelectable);
    return state;
}

QGraphicsEllipseItem* DrawHelper::createFinalState(QGraphicsScene* scene, qreal x, qreal y, qreal size)
{
    auto* outer = scene->addEllipse(x, y, size, size, QPen(Qt::black));
    scene->addEllipse(x + 1, y + 1, size - 2, size - 2, QPen(Qt::black));
    return outer;
}

QList<QGraphicsItem*> DrawHelper::createArrow(QGraphicsScene* scene,
    QGraphicsEllipseItem* from,
    QGraphicsEllipseItem* to,
    const QString& label)
{
    QList<QGraphicsItem*> items;

    QPointF p1 = from->sceneBoundingRect().center();
    QPointF p2 = to->sceneBoundingRect().center();

    auto* line = scene->addLine(QLineF(p1, p2), QPen(Qt::black, 2));
    items.append(line);

    QLineF linef(p1, p2);
    double angle = qDegreesToRadians(-linef.angle());

    qreal arrowSize = 12;
    QPointF arrowP1 = p2 + QPointF(cos(angle + 0.5) * -arrowSize, sin(angle + 0.5) * -arrowSize);
    QPointF arrowP2 = p2 + QPointF(cos(angle - 0.5) * -arrowSize, sin(angle - 0.5) * -arrowSize);

    auto* arrow = scene->addPolygon(QPolygonF({p2, arrowP1, arrowP2}),
                      QPen(Qt::black), QBrush(Qt::black));
    items.append(arrow);

    auto* text = scene->addText(label);
    text->setPos((p1.x() + p2.x()) / 2 - 10, (p1.y() + p2.y()) / 2 - 20);
    items.append(text);

    return items;
}

// ===============================
//   DiagramBuilder IMPLEMENTATION
// ===============================

DiagramElements DiagramBuilder::buildDynamicDiagram(QGraphicsScene* scene, const QString& tokenType)
{
    DiagramElements elements;
    NFA nfa;

    // Dynamically create the NFA based on tokenType
    if (tokenType == "Identifier") {
        nfa = buildIdentifierNFA();
    } else if (tokenType == "Number") {
        nfa = buildNumberNFA();
    } else if (tokenType == "String") {
        nfa = buildStringLiteralNFA("Print"); // Example string literal
    } else {
        nfa = buildSymbolNFA('a'); // Default fallback symbol
    }

    if (nfa.states.isEmpty()) {
        return elements;
    }

    // Dynamic layout: use a grid for better spacing
    int startX = 100;
    int startY = 100;
    int stepX = 120; // Horizontal step between states
    int stepY = 150; // Vertical step between states

    QMap<int, QPointF> statePositions;
    int rows = 3;  // Adjust number of rows based on the number of states
    int columns = (nfa.states.size() + rows - 1) / rows; // Calculate columns based on number of states

    for (int i = 0; i < nfa.states.size(); ++i) {
        int row = i / columns;
        int col = i % columns;

        statePositions[nfa.states[i].id] = QPointF(startX + col * stepX, startY + row * stepY);
    }

    // Draw states and transitions
    for (const auto& state : nfa.states) {
        QPointF pos = statePositions[state.id];
        QBrush brush = Qt::white;

        if (state.isAccept) {
            scene->addEllipse(pos.x() - 35, pos.y() - 35, 70, 70, QPen(QColor("#28a745"), 1), Qt::NoBrush);
            brush = QColor("#28a745");
        }
        if (state.id == nfa.startState) {
            brush = Qt::lightGray;
        }

        auto* circle = scene->addEllipse(pos.x() - 30, pos.y() - 30, 60, 60, QPen(Qt::black, 2), brush);
        auto* text = scene->addText(QString::number(state.id));
        text->setFont(QFont("Arial", 12, QFont::Bold));
        text->setPos(pos.x() - text->boundingRect().width() / 2,
                     pos.y() - text->boundingRect().height() / 2);
        elements.states[QString::number(state.id)] = circle;
    }

    // Draw transitions (arrows)
    for (const auto& trans : nfa.transitions) {
        if (!statePositions.contains(trans.from) || !statePositions.contains(trans.to)) continue;

        QPointF from = statePositions[trans.from];
        QPointF to = statePositions[trans.to];

        QLineF line(from, to);
        double angle = qDegreesToRadians(line.angle());
        QPointF p1 = to - QPointF(10 * cos(angle) - 5 * sin(angle), 10 * sin(angle) + 5 * cos(angle));
        QPointF p2 = to - QPointF(10 * cos(angle) + 5 * sin(angle), 10 * sin(angle) - 5 * cos(angle));

        scene->addLine(from.x(), from.y(), to.x(), to.y(), QPen(Qt::black, 2));
        scene->addLine(to.x(), to.y(), p1.x(), p1.y(), QPen(Qt::black, 2));
        scene->addLine(to.x(), to.y(), p2.x(), p2.y(), QPen(Qt::black, 2));

        QPointF labelPos = from * 0.7 + to * 0.3;
        auto* label = scene->addText(trans.symbol);
        label->setFont(QFont("Arial", 10));
        label->setDefaultTextColor(Qt::darkBlue);
        label->setPos(labelPos.x() - label->boundingRect().width() / 2,
                      labelPos.y() - label->boundingRect().height() / 2 - 10);
    }

    return elements;
}
// ==========================================================
//         LexicalAnalysisTab IMPLEMENTATION
// ==========================================================

LexicalAnalysisTab::LexicalAnalysisTab(QWidget* parent)
    : QWidget(parent), currentTokenIndex(0), currentStepIndex(0)
{
    int leftX = 20;
    int topY = 20;

    userlabel = new QLabel("Write your code here:", this);
    userlabel->setFont(QFont("Poppins", 14, QFont::Bold));
    userlabel->move(leftX, topY);

    userinput = new QTextEdit(this);
    userinput->setGeometry(leftX, 50, 900, 200);
    userinput->setFont(QFont("Consolas", 12));

    int runY = 50 + 225 + 10;
    run = new QPushButton("Run", this);
    run->setFont(QFont("Poppins", 10, QFont::Bold));
    run->setStyleSheet("background-color: #16163F; color: white;");
    run->setGeometry(leftX + 900 - 70, runY, 70, 30);
    connect(run, &QPushButton::clicked, this, &LexicalAnalysisTab::runLexicalAnalysis);

    int dfaY = runY + 40;
    dfa = new QLabel("DFA Diagram", this);
    dfa->setFont(QFont("Poppins", 14, QFont::Bold));
    dfa->move(leftX, dfaY);

    dfaScene = new QGraphicsScene(this);
    dfaView  = new QGraphicsView(dfaScene, this);
    dfaView->setGeometry(leftX, dfaY + 40, 900, 400);
    dfaView->setStyleSheet("background-color: dark gray; border: 1px solid #aaa;");
    dfaView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    dfaView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    dfaView->setDragMode(QGraphicsView::ScrollHandDrag);
    dfaView->setRenderHint(QPainter::Antialiasing);

    diagramElements = DiagramBuilder::buildExampleDiagram(dfaScene);

    QRectF diagramBounds = dfaScene->itemsBoundingRect();
    dfaScene->setSceneRect(diagramBounds);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    tokenlabel = new QLabel("Token Table", this);
    tokenlabel->setFont(QFont("Poppins", 14, QFont::Bold));
    rightLayout->addWidget(tokenlabel);

    tokenizationtable = new QTableWidget(this);
    tokenizationtable->setColumnCount(4);
    tokenizationtable->setHorizontalHeaderLabels({"Token", "Type", "Line", "Column"});
    tokenizationtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokenizationtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tokenizationtable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tokenizationtable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    rightLayout->addWidget(tokenizationtable);

    QWidget* rightContainer = new QWidget(this);
    rightContainer->setLayout(rightLayout);
    rightContainer->setGeometry(950, 20, 550, 750);

    // Animation timer
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &LexicalAnalysisTab::animateNextStep);

    // Connect tokens ready to start animation
    connect(this, &LexicalAnalysisTab::tokensReady, this, [this](const QList<QStringList>& tokens) {
        currentTokens = tokens;
        currentTokenIndex = 0;
        currentStepIndex = 0;
        currentSteps.clear();
        resetHighlighting();
        animationTimer->start(500); // 500ms per step
    });

    // Connect table item click to highlight token path
    connect(tokenizationtable, &QTableWidget::itemClicked, this, &LexicalAnalysisTab::onTokenClicked);
}

void LexicalAnalysisTab::resetHighlighting()
{
    // Reset all states to white
    for (auto* state : diagramElements.states.values()) {
        if (auto* ellipse = dynamic_cast<QGraphicsEllipseItem*>(state)) {
            ellipse->setBrush(QBrush(Qt::white));
        }
    }

    // Reset all transitions to black
    for (const auto& items : diagramElements.transitions.values()) {
        for (auto* item : items) {
            if (auto* line = dynamic_cast<QGraphicsLineItem*>(item)) {
                line->setPen(QPen(Qt::black, 2));
            } else if (auto* path = dynamic_cast<QGraphicsPathItem*>(item)) {
                path->setPen(QPen(Qt::black, 2));
            } else if (auto* poly = dynamic_cast<QGraphicsPolygonItem*>(item)) {
                poly->setPen(QPen(Qt::black));
                poly->setBrush(QBrush(Qt::black));
            }
        }
    }
}

void LexicalAnalysisTab::highlightState(QGraphicsEllipseItem* state)
{
    if (state) {
        state->setBrush(QBrush(QColor(255, 215, 0))); // Gold color
    }
}

void LexicalAnalysisTab::highlightTransition(const QList<QGraphicsItem*>& items)
{
    for (auto* item : items) {
        if (auto* line = dynamic_cast<QGraphicsLineItem*>(item)) {
            line->setPen(QPen(QColor(0, 150, 255), 3)); // Blue color
        } else if (auto* path = dynamic_cast<QGraphicsPathItem*>(item)) {
            path->setPen(QPen(QColor(0, 150, 255), 3));
        } else if (auto* poly = dynamic_cast<QGraphicsPolygonItem*>(item)) {
            poly->setPen(QPen(QColor(0, 150, 255)));
            poly->setBrush(QBrush(QColor(0, 150, 255)));
        }
    }
}

// Handle token click in table
void LexicalAnalysisTab::onTokenClicked(QTableWidgetItem* item)
{
    if (!item) return;

    // Stop animation if running
    if (animationTimer->isActive()) {
        animationTimer->stop();
    }

    int row = item->row();

    // Get token and type from the clicked row
    QString tokenText = tokenizationtable->item(row, 0)->text();
    QString tokenType = tokenizationtable->item(row, 1)->text();

    // Reset highlighting first
    resetHighlighting();

    // Get animation steps for this token
    QList<AnimationStep> steps = getAnimationSteps(tokenText, tokenType);

    // Highlight all states and transitions for this token
    QSet<QString> highlightedStates;

    for (const AnimationStep& step : steps) {
        // Highlight states
        if (diagramElements.states.contains(step.fromState)) {
            highlightState(diagramElements.states[step.fromState]);
            highlightedStates.insert(step.fromState);
        }
        if (diagramElements.states.contains(step.toState)) {
            highlightState(diagramElements.states[step.toState]);
            highlightedStates.insert(step.toState);
        }

        // Highlight transition
        if (!step.transitionKey.isEmpty() && diagramElements.transitions.contains(step.transitionKey)) {
            highlightTransition(diagramElements.transitions[step.transitionKey]);
        }
    }
}

QList<AnimationStep> LexicalAnalysisTab::getAnimationSteps(const QString& token, const QString& type)
{
    QList<AnimationStep> steps;

    if (type == "Keyword") {
        if (token == "def") {
            steps.append({"s0", "s2", "def_d"});
            steps.append({"s2", "s3", "def_e"});
            steps.append({"s3", "f4", "shared_f"});
        } else if (token == "elif") {
            steps.append({"s0", "s4", "shared_el_e"});
            steps.append({"s4", "s5", "shared_el_l"});
            steps.append({"s5", "s3", "elif_i"});
            steps.append({"s3", "f4", "shared_f"});
        } else if (token == "else") {
            steps.append({"s0", "s4", "shared_el_e"});
            steps.append({"s4", "s5", "shared_el_l"});
            steps.append({"s5", "s17", "else_s"});
            steps.append({"s17", "f4", "shared_final_e"});
        } else if (token == "for") {
            steps.append({"s0", "s6", "for_f"});
            steps.append({"s6", "s7", "for_o"});
            steps.append({"s7", "f4", "for_r"});
        } else if (token == "while") {
            steps.append({"s0", "s8", "while_w"});
            steps.append({"s8", "s9", "while_h"});
            steps.append({"s9", "s10", "while_i"});
            steps.append({"s10", "s17", "while_l"});
            steps.append({"s17", "f4", "shared_final_e"});
        } else if (token == "return") {
            steps.append({"s0", "s12", "return_r"});
            steps.append({"s12", "s13", "return_e"});
            steps.append({"s13", "s14", "return_t"});
            steps.append({"s14", "s15", "return_u"});
            steps.append({"s15", "s16", "return_r2"});
            steps.append({"s16", "f4", "return_n"});
        } else if (token == "if") {
            steps.append({"s0", "s3", "if_i"});
            steps.append({"s3", "f4", "shared_f"});
        } else {
            steps.append({"s0", "f4", "operator"});
        }
    } else if (type == "Number") {
        bool foundDot = false;
        bool inDecimalPart = false;

        for (int i = 0; i < token.length(); ++i) {
            QChar c = token[i];

            if (c == '.') {
                steps.append({"f1", "s1", "number_dot"});
                foundDot = true;
                inDecimalPart = true;
            } else if (c.isDigit()) {
                if (i == 0) {
                    steps.append({"s0", "f1", "number"});
                } else if (!foundDot) {
                    steps.append({"f1", "f1", "number_loop"});
                } else if (inDecimalPart && steps.isEmpty()) {
                    steps.append({"s1", "f2", "number_decimal"});
                    inDecimalPart = false;
                } else if (inDecimalPart) {
                    steps.append({"s1", "f2", "number_decimal"});
                    inDecimalPart = false;
                } else {
                    steps.append({"f2", "f2", "number_decimal_loop"});
                }
            }
        }
    } else if (type == "Identifier") {
        for (int i = 0; i < token.length(); ++i) {
            if (i == 0) {
                steps.append({"s0", "f3", "identifier"});
            } else {
                steps.append({"f3", "f3", "identifier_loop"});
            }
        }
    } else if (type == "Operator") {
        steps.append({"s0", "f4", "operator"});
    } else if (type == "Delimiter") {
        steps.append({"s0", "f4", "delimiters"});
    } else {
        steps.append({"s0", "s0", ""});
    }

    return steps;
}

void LexicalAnalysisTab::animateNextStep()
{
    // Check if we need to load a new token
    if (currentSteps.isEmpty() || currentStepIndex >= currentSteps.size()) {
        // Move to next token
        if (currentTokenIndex >= currentTokens.size()) {
            animationTimer->stop();
            resetHighlighting();
            return;
        }

        // Load next token
        QStringList token = currentTokens[currentTokenIndex];
        QString tokenText = token[0];
        QString tokenType = token[1];

        // Highlight current row in table
        tokenizationtable->selectRow(currentTokenIndex);

        // Get animation steps for this token
        currentSteps = getAnimationSteps(tokenText, tokenType);
        currentStepIndex = 0;
        currentTokenIndex++;

        resetHighlighting();
    }

    // Animate current step
    if (currentStepIndex < currentSteps.size()) {
        AnimationStep step = currentSteps[currentStepIndex];

        // Highlight states
        if (diagramElements.states.contains(step.fromState)) {
            highlightState(diagramElements.states[step.fromState]);
        }
        if (diagramElements.states.contains(step.toState)) {
            highlightState(diagramElements.states[step.toState]);
        }

        // Highlight transition
        if (!step.transitionKey.isEmpty() && diagramElements.transitions.contains(step.transitionKey)) {
            highlightTransition(diagramElements.transitions[step.transitionKey]);
        }

        currentStepIndex++;

        // If this was the last step of the token, pause a bit longer before next token
        if (currentStepIndex >= currentSteps.size()) {
            animationTimer->setInterval(800); // Longer pause between tokens
        } else {
            animationTimer->setInterval(500); // Normal speed between steps
        }
    }
}

// ================= RUN LEXICAL ANALYSIS ===================

void LexicalAnalysisTab::runLexicalAnalysis()
{
    tokenizationtable->setRowCount(0);
    QString code = userinput->toPlainText();
    QStringList lines = code.split('\n');

    QRegularExpression numberRegex(R"(^[0-9]+(\.[0-9]+)?)");
    QRegularExpression identifierRegex(R"(^[a-zA-Z_][a-zA-Z0-9_]*)");
    QStringList keywords = {"if", "elif", "else", "for", "while" "def", "return"};

    QRegularExpression operatorRegex(
        R"(^(==|!=|<=|>=|\*\*|//|\+=|-=|\*=|/=|\+|-|\*|/|%|=|<|>))"
    );

    QRegularExpression delimiterRegex(R"(^[\{\}\(\)\[\]:"'])");

    int row = 0;

    for (int lineNum = 0; lineNum < lines.size(); ++lineNum) {
        QString line = lines[lineNum];
        int col = 0;

        while (!line.isEmpty()) {
            int oldLen = line.length();
            line = line.trimmed();
            col += (oldLen - line.length());
            if (line.isEmpty()) break;

            QRegularExpressionMatch match;

            match = numberRegex.match(line);
            if (match.hasMatch()) {
                QString tok = match.captured();
                tokenizationtable->insertRow(row);
                tokenizationtable->setItem(row, 0, new QTableWidgetItem(tok));
                tokenizationtable->setItem(row, 1, new QTableWidgetItem("Number"));
                tokenizationtable->setItem(row, 2, new QTableWidgetItem(QString::number(lineNum + 1)));
                tokenizationtable->setItem(row, 3, new QTableWidgetItem(QString::number(col + 1))); // ✅ FIXED: Added Column

                line.remove(0, tok.length());
                col += tok.length();
                row++;
                continue;
            }

            match = delimiterRegex.match(line);
            if (match.hasMatch()) {
                QString tok = match.captured();
                tokenizationtable->insertRow(row);
                tokenizationtable->setItem(row, 0, new QTableWidgetItem(tok));
                tokenizationtable->setItem(row, 1, new QTableWidgetItem("Delimiter"));
                tokenizationtable->setItem(row, 2, new QTableWidgetItem(QString::number(lineNum + 1)));
                tokenizationtable->setItem(row, 3, new QTableWidgetItem(QString::number(col + 1))); // ✅ FIXED: Added Column

                line.remove(0, tok.length());
                col += tok.length();
                row++;
                continue;
            }

            match = operatorRegex.match(line);
            if (match.hasMatch()) {
                QString tok = match.captured();
                tokenizationtable->insertRow(row);
                tokenizationtable->setItem(row, 0, new QTableWidgetItem(tok));
                tokenizationtable->setItem(row, 1, new QTableWidgetItem("Operator"));
                tokenizationtable->setItem(row, 2, new QTableWidgetItem(QString::number(lineNum + 1)));
                tokenizationtable->setItem(row, 3, new QTableWidgetItem(QString::number(col + 1))); // ✅ FIXED: Added Column

                line.remove(0, tok.length());
                col += tok.length();
                row++;
                continue;
            }

            match = identifierRegex.match(line);
            if (match.hasMatch()) {
                QString tok = match.captured();

                tokenizationtable->insertRow(row);
                tokenizationtable->setItem(row, 0, new QTableWidgetItem(tok));

                if (keywords.contains(tok))
                    tokenizationtable->setItem(row, 1, new QTableWidgetItem("Keyword"));
                else
                    tokenizationtable->setItem(row, 1, new QTableWidgetItem("Identifier"));

                tokenizationtable->setItem(row, 2, new QTableWidgetItem(QString::number(lineNum + 1)));
                tokenizationtable->setItem(row, 3, new QTableWidgetItem(QString::number(col + 1))); // ✅ FIXED: Added Column

                line.remove(0, tok.length());
                col += tok.length();
                row++;
                continue;
            }

            tokenizationtable->insertRow(row);
            QString unknown = line.left(1);
            tokenizationtable->setItem(row, 0, new QTableWidgetItem(unknown));
            tokenizationtable->setItem(row, 1, new QTableWidgetItem("Unknown"));
            tokenizationtable->setItem(row, 2, new QTableWidgetItem(QString::number(lineNum + 1)));
            tokenizationtable->setItem(row, 3, new QTableWidgetItem(QString::number(col + 1))); // ✅ FIXED: Added Column

            line.remove(0, 1);
            col += 1;
            row++;
        }
    }

    QList<QStringList> tokensList;
    for (int r = 0; r < tokenizationtable->rowCount(); ++r) {
        QStringList t;
        t << tokenizationtable->item(r, 0)->text()
          << tokenizationtable->item(r, 1)->text()
          << tokenizationtable->item(r, 2)->text()
          << tokenizationtable->item(r, 3)->text();
        tokensList.append(t);
    }
    emit tokensReady(tokensList);
}
