#include "NFASimulatorTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QLabel>
#include <QFont>

NFASimulatorTab::NFASimulatorTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("NFA Diagram", this);
    titleLabel->setFont(QFont("Poppins", 16, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // 🔹 CORRECTED dropdown: 4 items matching 4 enum values
    patternSelector = new QComboBox(this);
    patternSelector->setFont(QFont("Poppins", 12));
    patternSelector->addItems({
        "Identifier: [a-zA-Z_][a-zA-Z0-9_]*",
        "Keywords: if, elif, else, for, while, def, return",
        "Integer: [0-9]+",
        "Delimeter: \"(\\.|[^\"])*\"",
    });
    patternSelector->setStyleSheet("padding: 8px;");
    layout->addWidget(patternSelector);

    graphicsView = new QGraphicsView(this);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setScene(new QGraphicsScene(this));
    graphicsView->scene()->setBackgroundBrush(QColor("#F8F9FA")); // Light gray background
    layout->addWidget(graphicsView, 4);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    drawButton = new QPushButton("Draw NFA", this);
    drawButton->setFont(QFont("Poppins", 10, QFont::Bold));
    drawButton->setStyleSheet("background-color: #16163F; color: white; padding: 11px 32px;");
    btnLayout->addStretch();
    btnLayout->addWidget(drawButton);
    layout->addLayout(btnLayout);

    setLayout(layout);

    connect(patternSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NFASimulatorTab::onPatternChanged);
    connect(drawButton, &QPushButton::clicked, this, &NFASimulatorTab::drawNFA);

    drawNFA();
}

void NFASimulatorTab::onPatternChanged(int index)
{
    currentPattern = static_cast<PatternType>(index);
    drawNFA();
}

void NFASimulatorTab::drawNFA()
{
    QGraphicsScene* scene = graphicsView->scene();
    scene->clear();

    QPen pen(Qt::black, 2);
    QBrush fillBrush(Qt::white);
    QBrush startBrush(Qt::lightGray);
    QBrush acceptBrush(QColor("#28a745")); // Green
    const QColor labelColor = Qt::darkBlue;

    auto drawState = [&](int x, int y, const QString& label, bool isStart, bool isAccept) {
        QGraphicsEllipseItem* circle = scene->addEllipse(x - 30, y - 30, 60, 60, pen,
            isAccept ? acceptBrush : (isStart ? startBrush : fillBrush));
        QGraphicsTextItem* text = scene->addText(label);
        text->setFont(QFont("Arial", 12, QFont::Bold));
        text->setPos(x - text->boundingRect().width() / 2,
                     y - text->boundingRect().height() / 2);
        return circle;
    };

    auto drawTransition = [&](QGraphicsEllipseItem* from, QGraphicsEllipseItem* to,
                              const QString& label, bool isLoop = false) {
        QRectF fromRect = from->boundingRect();
        QRectF toRect = to->boundingRect();
        QPointF fromCenter = from->pos() + fromRect.center();
        QPointF toCenter = to->pos() + toRect.center();

        if (isLoop) {
            QPainterPath path;
            path.moveTo(fromCenter);
            path.cubicTo(fromCenter + QPointF(40, -50),
                         fromCenter + QPointF(90, -50),
                         fromCenter + QPointF(90, 0));
            path.lineTo(fromCenter + QPointF(90, 10));
            path.cubicTo(fromCenter + QPointF(90, 70),
                         fromCenter + QPointF(40, 70),
                         fromCenter);
            scene->addPath(path, pen);

            QGraphicsTextItem* t = scene->addText(label);
            t->setFont(QFont("Arial", 10));
            t->setDefaultTextColor(labelColor);
            t->setPos(fromCenter.x() + 30, fromCenter.y() - 65);
            return;
        }

        QLineF line(fromCenter, toCenter);
        double angle = line.angle();
        double rad = qDegreesToRadians(angle);

        int arrowSize = 10;
        QPointF p1 = toCenter - QPointF(arrowSize * cos(rad) - 5 * sin(rad),
                                        arrowSize * sin(rad) + 5 * cos(rad));
        QPointF p2 = toCenter - QPointF(arrowSize * cos(rad) + 5 * sin(rad),
                                        arrowSize * sin(rad) - 5 * cos(rad));

        scene->addLine(fromCenter.x(), fromCenter.y(),
                       toCenter.x(), toCenter.y(), pen);
        scene->addLine(toCenter.x(), toCenter.y(), p1.x(), p1.y(), pen);
        scene->addLine(toCenter.x(), toCenter.y(), p2.x(), p2.y(), pen);

        QPointF mid = (fromCenter + toCenter) / 2.0;
        QGraphicsTextItem* t = scene->addText(label);
        t->setFont(QFont("Arial", 10));
        t->setDefaultTextColor(labelColor);
        t->setPos(mid.x() - t->boundingRect().width() / 2,
                  mid.y() - t->boundingRect().height() / 2 - 15);
    };

    switch (currentPattern) {
    case Identifier: {
        int x0 = 120, y0 = 200;
        int x1 = 320, y1 = 200;
        int x2 = 520, y2 = 200;

        auto q0 = drawState(x0, y0, "q0", true, false);
        auto q1 = drawState(x1, y1, "q1", false, false);
        auto q2 = drawState(x2, y2, "q2", false, true);

        drawTransition(q0, q1, "[a-zA-Z_]");
        drawTransition(q1, q1, "[a-zA-Z0-9_]", true);
        drawTransition(q1, q2, "ε");

        QGraphicsTextItem* title = scene->addText("NFA for Identifier");
        title->setDefaultTextColor(Qt::black);
        title->setPos(100, 80);
        break;
    }

    // 🔹 NEW: Keywords NFA (Trie-like structure)
    case Keywords: {
        // Layout positions
        int x0 = 200, y0 = 200; // start

        int x_d = 320, y_d = 120; // 'd' for def
        int x_e = 320, y_e = 200; // 'e' for elif/else
        int x_f = 320, y_f = 280; // 'f' for for
        int x_i = 320, y_i = 360; // 'i' for if
        int x_r = 320, y_r = 440; // 'r' for return
        int x_w = 320, y_w = 520; // 'w' for while

        auto q0 = drawState(x0, y0, "q0", true, false);

        // First letter branches
        auto q_d = drawState(x_d, y_d, "d", false, false);
        auto q_e = drawState(x_e, y_e, "e", false, false);
        auto q_f = drawState(x_f, y_f, "f", false, false);
        auto q_i = drawState(x_i, y_i, "i", false, false);
        auto q_r = drawState(x_r, y_r, "r", false, false);
        auto q_w = drawState(x_w, y_w, "w", false, false);

        drawTransition(q0, q_d, "d");
        drawTransition(q0, q_e, "e");
        drawTransition(q0, q_f, "f");
        drawTransition(q0, q_i, "i");
        drawTransition(q0, q_r, "r");
        drawTransition(q0, q_w, "w");

        // def: d → e → f (accept)
        auto q_def_e = drawState(x_d + 120, y_d, "e", false, false);
        auto q_def_f = drawState(x_d + 240, y_d, "f", false, true);
        drawTransition(q_d, q_def_e, "e");
        drawTransition(q_def_e, q_def_f, "f");

        // elif/else: e → l → (i → f) or (s → e)
        auto q_el_l = drawState(x_e + 120, y_e, "l", false, false);
        drawTransition(q_e, q_el_l, "l");

        auto q_elif_i = drawState(x_e + 240, y_e - 30, "i", false, false);
        auto q_elif_f = drawState(x_e + 360, y_e - 30, "f", false, true);
        drawTransition(q_el_l, q_elif_i, "i");
        drawTransition(q_elif_i, q_elif_f, "f");

        auto q_else_s = drawState(x_e + 240, y_e + 30, "s", false, false);
        auto q_else_e = drawState(x_e + 360, y_e + 30, "e", false, true);
        drawTransition(q_el_l, q_else_s, "s");
        drawTransition(q_else_s, q_else_e, "e");

        // for: f → o → r (accept)
        auto q_for_o = drawState(x_f + 120, y_f, "o", false, false);
        auto q_for_r = drawState(x_f + 240, y_f, "r", false, true);
        drawTransition(q_f, q_for_o, "o");
        drawTransition(q_for_o, q_for_r, "r");

        // if: i → f (accept)
        auto q_if_f = drawState(x_i + 120, y_i, "f", false, true);
        drawTransition(q_i, q_if_f, "f");

        // return: r → e → t → u → r → n (accept)
        auto q_ret_e = drawState(x_r + 120, y_r, "e", false, false);
        auto q_ret_t = drawState(x_r + 240, y_r, "t", false, false);
        auto q_ret_u = drawState(x_r + 360, y_r, "u", false, false);
        auto q_ret_r = drawState(x_r + 480, y_r, "r", false, false);
        auto q_ret_n = drawState(x_r + 600, y_r, "n", false, true);
        drawTransition(q_r, q_ret_e, "e");
        drawTransition(q_ret_e, q_ret_t, "t");
        drawTransition(q_ret_t, q_ret_u, "u");
        drawTransition(q_ret_u, q_ret_r, "r");
        drawTransition(q_ret_r, q_ret_n, "n");

        // while: w → h → i → l → e (accept)
        auto q_wh_h = drawState(x_w + 120, y_w, "h", false, false);
        auto q_wh_i = drawState(x_w + 240, y_w, "i", false, false);
        auto q_wh_l = drawState(x_w + 360, y_w, "l", false, false);
        auto q_wh_e = drawState(x_w + 480, y_w, "e", false, true);
        drawTransition(q_w, q_wh_h, "h");
        drawTransition(q_wh_h, q_wh_i, "i");
        drawTransition(q_wh_i, q_wh_l, "l");
        drawTransition(q_wh_l, q_wh_e, "e");

        QGraphicsTextItem* title = scene->addText("NFA for Keywords");
        title->setDefaultTextColor(Qt::black);
        title->setPos(100, 80);
        break;
    }

    case Integer: {
        int x0 = 150, y0 = 200;
        int x1 = 300, y1 = 200;
        int x2 = 450, y2 = 200;

        auto q0 = drawState(x0, y0, "q0", true, false);
        auto q1 = drawState(x1, y1, "q1", false, false);
        auto q2 = drawState(x2, y2, "q2", false, true);

        drawTransition(q0, q1, "'-'");
        drawTransition(q0, q2, "[0-9]");
        drawTransition(q1, q2, "[0-9]");
        drawTransition(q2, q2, "[0-9]", true);

        QGraphicsTextItem* title = scene->addText("NFA for Integers");
        title->setDefaultTextColor(Qt::black);
        title->setPos(100, 80);
        break;
    }

    case StringLiteral: {
        int x0 = 150, y0 = 200;
        int x1 = 320, y1 = 200;
        int x2 = 490, y2 = 200;

        auto q0 = drawState(x0, y0, "q0", true, false);
        auto q1 = drawState(x1, y1, "q1", false, false);
        auto q2 = drawState(x2, y2, "q2", false, true);

        drawTransition(q0, q1, "\"");
        drawTransition(q1, q1, "[^\\\"]", true);
        drawTransition(q1, q2, "\"");

        QGraphicsTextItem* title = scene->addText("NFA for Strings");
        title->setDefaultTextColor(Qt::black);
        title->setPos(100, 80);
        break;
    }
    }

    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-50, -80, 50, 50));
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}