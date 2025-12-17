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
    scene->addEllipse(x + 10, y + 10, size - 20, size - 20, QPen(Qt::black));
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
    text->setPos((p1.x()+p2.x())/2 - 10, (p1.y()+p2.y())/2 - 20);
    items.append(text);

    return items;
}

QList<QGraphicsItem*> DrawHelper::createCurvedArrow(QGraphicsScene* scene,
    QGraphicsEllipseItem* from,
    QGraphicsEllipseItem* to,
    const QString& label,
    qreal curveOffset,
    qreal labelOffsetY)
{
    QList<QGraphicsItem*> items;

    QPointF p1 = from->sceneBoundingRect().center();
    QPointF p2 = to->sceneBoundingRect().center();

    QPointF mid = (p1 + p2) / 2;
    QPointF control(mid.x(), mid.y() - curveOffset);

    QPainterPath path(p1);
    path.quadTo(control, p2);

    auto* pathItem = scene->addPath(path, QPen(Qt::black, 2));
    items.append(pathItem);

    QLineF tangent(control, p2);
    double angle = qDegreesToRadians(-tangent.angle());

    qreal arrowSize = 12;
    QPointF arrowP1 = p2 + QPointF(cos(angle + 0.5) * -arrowSize, sin(angle + 0.5) * -arrowSize);
    QPointF arrowP2 = p2 + QPointF(cos(angle - 0.5) * -arrowSize, sin(angle - 0.5) * -arrowSize);

    auto* arrow = scene->addPolygon(QPolygonF({p2, arrowP1, arrowP2}),
                      QPen(Qt::black), QBrush(Qt::black));
    items.append(arrow);

    auto* text = scene->addText(label);
    text->setPos(control.x() - 10, control.y() - 20);
    items.append(text);

    return items;
}

QList<QGraphicsItem*> DrawHelper::createSelfLoop(QGraphicsScene* scene,
    QGraphicsEllipseItem* state,
    const QString& label)
{
    QList<QGraphicsItem*> items;

    QRectF r = state->sceneBoundingRect();
    QPointF topCenter(r.center().x(), r.top());

    QPointF c1(topCenter.x() - 40, topCenter.y() - 60);
    QPointF c2(topCenter.x() + 40, topCenter.y() - 60);

    QPainterPath path(topCenter);
    path.cubicTo(c1, c2, topCenter);

    auto* pathItem = scene->addPath(path, QPen(Qt::black, 2));
    items.append(pathItem);

    QPointF end = topCenter;
    QPointF left(end.x() - 10, end.y() - 5);
    QPointF right(end.x() + 10, end.y() - 5);

    auto* arrow = scene->addPolygon(QPolygonF({end, left, right}),
                      QPen(Qt::black), QBrush(Qt::black));
    items.append(arrow);

    auto* text = scene->addText(label);
    text->setPos(topCenter.x() - 10, topCenter.y() - 80);
    items.append(text);

    return items;
}

// ===============================
//   DiagramBuilder IMPLEMENTATION
// ===============================

DiagramElements DiagramBuilder::buildExampleDiagram(QGraphicsScene* scene)
{
    DiagramElements elements;

    auto s0 = DrawHelper::createState(scene, -400, 1000);
    elements.states["s0"] = s0;

    auto f1 = DrawHelper::createFinalState(scene, -250, 850);
    elements.states["f1"] = f1;
    auto s1 = DrawHelper::createState(scene, -150, 850);
    elements.states["s1"] = s1;
    auto f2 = DrawHelper::createFinalState(scene, 10, 850);
    elements.states["f2"] = f2;

    auto f3 = DrawHelper::createFinalState(scene, -250, 970);
    elements.states["f3"] = f3;

    auto s2 = DrawHelper::createState(scene, -150, 1025);
    elements.states["s2"] = s2;
    auto s3 = DrawHelper::createState(scene, 10, 1000);
    elements.states["s3"] = s3;

    auto s4 = DrawHelper::createState(scene, -250, 1100);
    elements.states["s4"] = s4;
    auto s5 = DrawHelper::createState(scene, -150, 1100);
    elements.states["s5"] = s5;

    auto s6 = DrawHelper::createState(scene, 10, 1150);
    elements.states["s6"] = s6;
    auto s7 = DrawHelper::createState(scene, 100, 1150);
    elements.states["s7"] = s7;

    auto s8 = DrawHelper::createState(scene, -350, 1200);
    elements.states["s8"] = s8;
    auto s9 = DrawHelper::createState(scene, -250, 1200);
    elements.states["s9"] = s9;
    auto s10 = DrawHelper::createState(scene, -150, 1200);
    elements.states["s10"] = s10;

    auto s12 = DrawHelper::createState(scene, -350, 1250);
    elements.states["s12"] = s12;
    auto s13 = DrawHelper::createState(scene, -250, 1250);
    elements.states["s13"] = s13;
    auto s14 = DrawHelper::createState(scene, -150, 1250);
    elements.states["s14"] = s14;
    auto s15 = DrawHelper::createState(scene, 10, 1250);
    elements.states["s15"] = s15;
    auto s16 = DrawHelper::createState(scene, 100, 1250);
    elements.states["s16"] = s16;

    // NEW: State for 'else' keyword
    auto s17 = DrawHelper::createState(scene, -70, 1150);
    elements.states["s17"] = s17;

    auto f4 = DrawHelper::createFinalState(scene, 300, 900);
    elements.states["f4"] = f4;

    // Transitions
    elements.transitions["number"] = DrawHelper::createArrow(scene, s0, f1, "0-9");
    elements.transitions["number_loop"] = DrawHelper::createSelfLoop(scene, f1, "0-9");
    elements.transitions["number_dot"] = DrawHelper::createArrow(scene, f1, s1, ".");
    elements.transitions["number_decimal"] = DrawHelper::createArrow(scene, s1, f2, "0-9");
    elements.transitions["number_decimal_loop"] = DrawHelper::createSelfLoop(scene, f2, "0-9");

    elements.transitions["identifier"] = DrawHelper::createArrow(scene, s0, f3, "a-z, A-Z, _");
    elements.transitions["identifier_loop"] = DrawHelper::createSelfLoop(scene, f3, "a-z, A-Z, 0-9, _");

    elements.transitions["def_d"] = DrawHelper::createArrow(scene, s0, s2, "d");
    elements.transitions["def_e"] = DrawHelper::createArrow(scene, s2, s3, "e");
    elements.transitions["shared_f"] = DrawHelper::createArrow(scene, s3, f4, "f");

    elements.transitions["shared_el_e"] = DrawHelper::createArrow(scene, s0, s4, "e");
    elements.transitions["shared_el_l"] = DrawHelper::createArrow(scene, s4, s5, "l");

    elements.transitions["elif_i"] = DrawHelper::createArrow(scene, s5, s3, "i");
    elements.transitions["else_s"] = DrawHelper::createArrow(scene, s5, s17, "s");
    elements.transitions["shared_final_e"] = DrawHelper::createArrow(scene, s17, f4, "e");

    elements.transitions["if_i"] = DrawHelper::createArrow(scene, s0, s3, "i");

    elements.transitions["for_f"] = DrawHelper::createArrow(scene, s0, s6, "f");
    elements.transitions["for_o"] = DrawHelper::createArrow(scene, s6, s7, "o");
    elements.transitions["for_r"] = DrawHelper::createArrow(scene, s7, f4, "r");

    elements.transitions["while_w"] = DrawHelper::createArrow(scene, s0, s8, "w");
    elements.transitions["while_h"] = DrawHelper::createArrow(scene, s8, s9, "h");
    elements.transitions["while_i"] = DrawHelper::createArrow(scene, s9, s10, "i");
    elements.transitions["while_l"] = DrawHelper::createArrow(scene, s10, s17, "l");

    elements.transitions["return_r"] = DrawHelper::createArrow(scene, s0, s12, "r");
    elements.transitions["return_e"] = DrawHelper::createArrow(scene, s12, s13, "e");
    elements.transitions["return_t"] = DrawHelper::createArrow(scene, s13, s14, "t");
    elements.transitions["return_u"] = DrawHelper::createArrow(scene, s14, s15, "u");
    elements.transitions["return_r2"] = DrawHelper::createArrow(scene, s15, s16, "r");
    elements.transitions["return_n"] = DrawHelper::createArrow(scene, s16, f4, "n");

    elements.transitions["operator"] = DrawHelper::createCurvedArrow(scene, s0, f4,
        "+, -, *, /, %, **, //=, +=, -=, *=, /===, !=, <=, >=, <, >, =");
    elements.transitions["delimiters"] = DrawHelper::createArrow(scene, s0, f4, "{,},(,),[,],:,\",\'");

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