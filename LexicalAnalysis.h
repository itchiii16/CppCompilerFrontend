#ifndef AUTOMATATOOLS_H
#define AUTOMATATOOLS_H


#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QtMath>


// ==========================
//        DrawHelper
// ==========================
class DrawHelper {
public:
    static QGraphicsEllipseItem* createState(
        QGraphicsScene* scene, qreal x, qreal y, qreal size = 30
    );


    static QGraphicsEllipseItem* createFinalState(
        QGraphicsScene* scene, qreal x, qreal y, qreal size = 30
    );


    static QList<QGraphicsItem*> createArrow(
        QGraphicsScene* scene,
        QGraphicsEllipseItem* from,
        QGraphicsEllipseItem* to,
        const QString& label
    );


    static QList<QGraphicsItem*> createCurvedArrow(
        QGraphicsScene* scene,
        QGraphicsEllipseItem* from,
        QGraphicsEllipseItem* to,
        const QString& label,
        qreal curveOffset = 70,
        qreal labelOffsetY = 2000
    );


    static QList<QGraphicsItem*> createSelfLoop(
        QGraphicsScene* scene,
        QGraphicsEllipseItem* state,
        const QString& label
    );
};


// ==========================
//      DiagramBuilder
// ==========================
struct DiagramElements {
    QMap<QString, QGraphicsEllipseItem*> states;
    QMap<QString, QList<QGraphicsItem*>> transitions;
};


class DiagramBuilder
{
public:
    static DiagramElements buildExampleDiagram(QGraphicsScene* scene);
};


// ==========================
//   Animation Step Structure
// ==========================
struct AnimationStep {
    QString fromState;
    QString toState;
    QString transitionKey;
};


// ==========================
//    LexicalAnalysisTab
// ==========================
class LexicalAnalysisTab : public QWidget
{
    Q_OBJECT


public:
    LexicalAnalysisTab(QWidget* parent = nullptr);
    QTableWidget* getTokenizationTable() const { return tokenizationtable; }


signals:
    void tokensReady(const QList<QStringList>& tokens);


private slots:
    void runLexicalAnalysis();
    void animateNextStep();
    void onTokenClicked(QTableWidgetItem* item);  // NEW: Handle token click


private:
    void resetHighlighting();
    void highlightState(QGraphicsEllipseItem* state);
    void highlightTransition(const QList<QGraphicsItem*>& items);
    QList<AnimationStep> getAnimationSteps(const QString& token, const QString& type);


    QLabel* userlabel;
    QTextEdit* userinput;
    QPushButton* run;


    QLabel* dfa;
    QGraphicsScene* dfaScene;
    QGraphicsView* dfaView;


    QLabel* tokenlabel;
    QTableWidget* tokenizationtable;


    // Animation members
    DiagramElements diagramElements;
    QTimer* animationTimer;
    QList<QStringList> currentTokens;
    int currentTokenIndex;
    QList<AnimationStep> currentSteps;
    int currentStepIndex;
    QString currentAnimatingState;
};


#endif