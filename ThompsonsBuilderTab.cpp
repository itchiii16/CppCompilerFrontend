#include "ThompsonsBuilderTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>
#include <QLabel>
#include <QFont>
#include <QStack>
#include <QMap>
#include <QDebug>

ThompsonsBuilderTab::ThompsonsBuilderTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Thompson's NFA Builder", this);
    titleLabel->setFont(QFont("Poppins", 16, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel* inputLabel = new QLabel("Enter regex (e.g., a*b|c):", this);
    inputLabel->setFont(QFont("Poppins", 12));
    layout->addWidget(inputLabel);

    regexInput = new QLineEdit(this);
    regexInput->setFont(QFont("Consolas", 12));
    regexInput->setPlaceholderText("Try: a, a*, ab, a|b, a*b|c");
    regexInput->setText("a|b");
    connect(regexInput, &QLineEdit::textChanged, this, [this]() {
        buildNFA();
    });
    layout->addWidget(regexInput);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    buildButton = new QPushButton("Build NFA", this);
    buildButton->setFont(QFont("Poppins", 10, QFont::Bold));
    buildButton->setStyleSheet("background-color: #16163F; color: white; padding: 11px 28px;");
    btnLayout->addStretch();
    btnLayout->addWidget(buildButton);
    layout->addLayout(btnLayout);

    buildLog = new QTextEdit(this);
    buildLog->setFont(QFont("Consolas", 10));
    buildLog->setReadOnly(true);
    buildLog->setFixedHeight(60);
    buildLog->setPlaceholderText("Status...");
    layout->addWidget(buildLog);

    stepLog = new QTextEdit(this);
    stepLog->setFont(QFont("Consolas", 10));
    stepLog->setReadOnly(true);
    stepLog->setFixedHeight(100);
    stepLog->setPlaceholderText("Construction steps...");
    layout->addWidget(stepLog);

    graphicsView = new QGraphicsView(this);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setScene(new QGraphicsScene(this));
    graphicsView->scene()->setBackgroundBrush(QColor("#FAFAFA"));
    layout->addWidget(graphicsView, 4);

    setLayout(layout);
    connect(buildButton, &QPushButton::clicked, this, &ThompsonsBuilderTab::buildNFA);
}

// --- NFA Construction (unchanged) ---
ThompsonsBuilderTab::NFA ThompsonsBuilderTab::createSymbolNFA(const QString& symbol)
{
    NFA nfa;
    nfa.states = { {0, false}, {1, true} };
    nfa.transitions = { {0, symbol, 1} };
    nfa.startState = 0;
    nfa.acceptState = 1;
    return nfa;
}

ThompsonsBuilderTab::NFA ThompsonsBuilderTab::unionNFA(const NFA& n1, const NFA& n2)
{
    NFA result;
    int nextId = 0;
    result.states.append({nextId++, false}); // new start
    result.states.append({nextId++, true});  // new accept
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
    for (const auto& s : n1.states) if (s.isAccept) result.transitions.append({idMap[s.id], "ε", newAccept});
    for (const auto& s : n2.states) if (s.isAccept) result.transitions.append({idMap[s.id], "ε", newAccept});
    for (const auto& t : n1.transitions) result.transitions.append({idMap[t.from], t.symbol, idMap[t.to]});
    for (const auto& t : n2.transitions) result.transitions.append({idMap[t.from], t.symbol, idMap[t.to]});

    result.startState = newStart;
    result.acceptState = newAccept;
    return result;
}

ThompsonsBuilderTab::NFA ThompsonsBuilderTab::concatNFA(const NFA& n1, const NFA& n2)
{
    NFA result = n1;
    int offset = n1.states.size();
    for (const auto& s : n2.states) result.states.append({s.id + offset, s.isAccept});
    for (auto& s : result.states) if (s.id == n1.acceptState) s.isAccept = false;
    result.transitions.append({n1.acceptState, "ε", n2.startState + offset});
    for (const auto& t : n2.transitions) result.transitions.append({t.from + offset, t.symbol, t.to + offset});
    result.acceptState = n2.acceptState + offset;
    return result;
}

ThompsonsBuilderTab::NFA ThompsonsBuilderTab::closureNFA(const NFA& n)
{
    NFA result;
    int nextId = 0;
    result.states.append({nextId++, false});
    result.states.append({nextId++, true});
    int newStart = result.states[0].id;
    int newAccept = result.states[1].id;

    QMap<int, int> idMap;
    for (const auto& s : n.states) { idMap[s.id] = nextId++; result.states.append({idMap[s.id], s.isAccept}); }

    result.transitions.append({newStart, "ε", idMap[n.startState]});
    result.transitions.append({newStart, "ε", newAccept});
    for (const auto& s : n.states) {
        if (s.isAccept) {
            result.transitions.append({idMap[s.id], "ε", idMap[n.startState]});
            result.transitions.append({idMap[s.id], "ε", newAccept});
            for (auto& rs : result.states) if (rs.id == idMap[s.id]) rs.isAccept = false;
        }
    }
    for (const auto& t : n.transitions) result.transitions.append({idMap[t.from], t.symbol, idMap[t.to]});

    result.startState = newStart;
    result.acceptState = newAccept;
    return result;
}

// --- NFA Building (unchanged) ---
ThompsonsBuilderTab::NFA ThompsonsBuilderTab::buildFromRegex(const QString& regex)
{
    buildSteps.clear();
    if (regex.isEmpty()) {
        NFA nfa;
        nfa.states = {{0, true}};
        nfa.startState = 0;
        nfa.acceptState = 0;
        buildSteps.append({"Empty regex"});
        return nfa;
    }

    QStack<NFA> operandStack;
    QStack<QChar> operatorStack;

    auto applyOperator = [&]() {
        if (operatorStack.isEmpty()) return;
        QChar op = operatorStack.pop();
        if (op == '|') {
            if (operandStack.size() >= 2) {
                NFA n2 = operandStack.pop();
                NFA n1 = operandStack.pop();
                buildSteps.append({"Union: " + operandStack.empty() ? "" : ""}); // simplified
                operandStack.push(unionNFA(n1, n2));
            }
        }
    };

    for (int i = 0; i < regex.length(); ++i) {
        QChar c = regex[i];
        if (c.isLetterOrNumber() || c == '_') {
            operandStack.push(createSymbolNFA(QString(c)));
            if (i + 1 < regex.length()) {
                QChar next = regex[i + 1];
                if (next.isLetterOrNumber() || next == '_' || next == '(' || next == '*') {
                    operatorStack.push('.');
                }
            }
        } else if (c == '*') {
            if (!operandStack.isEmpty()) {
                NFA n = operandStack.pop();
                operandStack.push(closureNFA(n));
            }
        } else if (c == '|') {
            applyOperator();
            operatorStack.push('|');
        } else if (c == '.') {
            operatorStack.push('.');
        }
    }
    applyOperator();

    while (operandStack.size() > 1) {
        NFA n2 = operandStack.pop();
        NFA n1 = operandStack.pop();
        operandStack.push(concatNFA(n1, n2));
    }

    return operandStack.isEmpty() ? createSymbolNFA("ε") : operandStack.top();
}

// --- DRAWING WITH STRUCTURED LAYOUT ---
void ThompsonsBuilderTab::drawNFA(const NFA& nfa)
{
    QGraphicsScene* scene = graphicsView->scene();
    scene->clear();

    // Special layout for union (a|b)
    bool isUnion = false;
    int forkState = -1, joinState = -1;
    QMap<int, QList<int>> outTrans, inTrans;

    for (const auto& t : nfa.transitions) {
        outTrans[t.from].append(t.to);
        inTrans[t.to].append(t.from);
        if (t.symbol == "ε") {
            // Detect fork: state with 2+ outgoing ε
            if (outTrans[t.from].size() >= 2) {
                isUnion = true;
                forkState = t.from;
            }
            // Detect join: state with 2+ incoming ε
            if (inTrans[t.to].size() >= 2) {
                joinState = t.to;
            }
        }
    }

    // Assign positions
    QMap<int, QPointF> statePositions;
    int startX = 100, startY = 200;

    if (isUnion && forkState != -1 && joinState != -1) {
        // Union layout: two branches
        statePositions[nfa.startState] = QPointF(startX, startY);
        statePositions[forkState] = QPointF(startX + 120, startY);

        // Find left and right branches
        int leftBranch = -1, rightBranch = -1;
        for (int to : outTrans[forkState]) {
            if (to != joinState) {
                if (leftBranch == -1) leftBranch = to;
                else rightBranch = to;
            }
        }

        if (leftBranch != -1) statePositions[leftBranch] = QPointF(startX + 240, startY - 60);
        if (rightBranch != -1) statePositions[rightBranch] = QPointF(startX + 240, startY + 60);
        if (joinState != -1) statePositions[joinState] = QPointF(startX + 360, startY);
        statePositions[nfa.acceptState] = QPointF(startX + 480, startY);
    } else {
        // Default linear layout
        int step = 140;
        for (int i = 0; i < nfa.states.size(); ++i) {
            statePositions[nfa.states[i].id] = QPointF(startX + i * step, startY);
        }
    }

    // Draw states
    QMap<int, QGraphicsEllipseItem*> stateItems;
    for (const auto& state : nfa.states) {
        if (!statePositions.contains(state.id)) continue;
        QPointF pos = statePositions[state.id];
        QBrush brush = state.id == nfa.startState ? QColor("#E9ECEF") :
                      state.isAccept ? QColor("#28A745") : Qt::white;

        if (state.isAccept) {
            scene->addEllipse(pos.x()-35, pos.y()-35, 70, 70, QPen(QColor("#28A745"), 1), Qt::NoBrush);
        }
        auto circle = scene->addEllipse(pos.x()-30, pos.y()-30, 60, 60, QPen(Qt::black, 2), brush);
        auto text = scene->addText(QString::number(state.id));
        text->setFont(QFont("Arial", 12, QFont::Bold));
        text->setPos(pos.x() - text->boundingRect().width()/2,
                     pos.y() - text->boundingRect().height()/2);
        stateItems[state.id] = circle;
    }

    // Draw transitions
    for (const auto& trans : nfa.transitions) {
        if (!stateItems.contains(trans.from) || !stateItems.contains(trans.to)) continue;
        QPointF from = statePositions[trans.from];
        QPointF to = statePositions[trans.to];

        // Straight line
        scene->addLine(from.x(), from.y(), to.x(), to.y(), QPen(Qt::black, 2));

        // Arrowhead
        QLineF line(from, to);
        double angle = qDegreesToRadians(line.angle());
        QPointF p1 = to - QPointF(10*cos(angle) - 5*sin(angle), 10*sin(angle) + 5*cos(angle));
        QPointF p2 = to - QPointF(10*cos(angle) + 5*sin(angle), 10*sin(angle) - 5*cos(angle));
        scene->addLine(to.x(), to.y(), p1.x(), p1.y(), QPen(Qt::black, 2));
        scene->addLine(to.x(), to.y(), p2.x(), p2.y(), QPen(Qt::black, 2));

        // Label near 'from'
        QPointF labelPos = from * 0.75 + to * 0.25;
        auto label = scene->addText(trans.symbol);
        label->setFont(QFont("Arial", 11));
        label->setDefaultTextColor(Qt::darkBlue);
        label->setPos(labelPos.x() - label->boundingRect().width()/2,
                      labelPos.y() - label->boundingRect().height()/2 - 10);
    }

    // Title
    //auto title = scene->addText("Thompson's NFA Construction");
    //title->setFont(QFont("Poppins", 16, QFont::Bold));
    //title->setPos(startX - 20, startY - 100);
    //scene->setSceneRect(scene->itemsBoundingRect().adjusted(-40, -100, 40, 40));
    //graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void ThompsonsBuilderTab::buildNFA()
{
    QString regex = regexInput->text().trimmed();
    if (regex.isEmpty()) {
        graphicsView->scene()->clear();
        buildLog->setPlainText("Enter a regex to build NFA.");
        stepLog->clear();
        return;
    }

    buildLog->setPlainText("Building NFA for: " + regex);
    NFA nfa = buildFromRegex(regex);

    // Update steps
    QStringList steps;
    for (const auto& s : buildSteps) steps << "• " + s.description;
    stepLog->setPlainText(steps.join("\n"));

    drawNFA(nfa);
}