#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>

// Forward declarations
class ProjectOverviewTab;
class LexicalAnalysisTab;
class SyntaxAnalysisTab;
class NFASimulatorTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QTabWidget* tabWidget;
    ProjectOverviewTab* overviewTab;
    LexicalAnalysisTab* lexicalTab;
    SyntaxAnalysisTab* syntaxTab;
};

#endif // MAINWINDOW_H