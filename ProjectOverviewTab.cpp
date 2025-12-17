#include "ProjectOverviewTab.h"
#include <QVBoxLayout>
#include <QFont>
#include <QTextCursor>
#include <QTextCharFormat>

ProjectOverviewTab::ProjectOverviewTab(QWidget* parent)
    : QWidget(parent)
{
    content = new QTextEdit(this);
    content->setReadOnly(true);
    content->setFont(QFont("Poppins", 12));
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QTextCursor cursor(content->document());
    QTextCharFormat titleFormat;
    titleFormat.setFont(QFont("Poppins", 20, QFont::Bold));

    QTextCharFormat subtitleFormat;
    subtitleFormat.setFont(QFont("Poppins", 18, QFont::Bold));

    QTextCharFormat normalFormat;
    normalFormat.setFont(QFont("Poppins", 15));

    // Insert main title
    cursor.insertText("\n\nCS311: Automata Theory and Formal Languages\n\n", titleFormat);

    // Project Title
    cursor.insertText("\nProject Title:\n", subtitleFormat);
    cursor.insertText("C++ Compiler Front-End Simulator for Lexical and Syntactic Analysis\n\n\n\n", normalFormat);

    cursor.insertText("Section:\n", subtitleFormat);
    cursor.insertText("CS3A\n\n", normalFormat);

        cursor.insertText("Team Members:\n", subtitleFormat);
    cursor.insertText("Adanza, Aaron\nGultiano, Kathleen Grace\nJison, Remar\nLaplap, Mariel\n\n\n\n", normalFormat);

    // Description
    cursor.insertText("Description:\n", subtitleFormat);
    cursor.insertText(
        "This project implements a C++ Compiler Front-End Simulator with lexical and syntax validation. "
        "It demonstrates key concepts from Automata Theory by simulating the behavior of finite automata (FA) for lexical analysis "
        "and pushdown automata (PDA) for syntax validation.\n\n", normalFormat);

    // Components
    cursor.insertText("Components:\n", subtitleFormat);
    cursor.insertText(
        "1. Lexical Analysis\n"
        "   - Scans multi-line input code to identify tokens: Numbers, Identifiers, Keywords, Operators, and Delimiters.\n"
        "   - Tokens are recognized using regular expressions and modeled via a minimized Deterministic Finite Automaton (mDFA).\n"
        "   - The GUI displays the token table and highlights mDFA states step-by-step during scanning.\n\n", normalFormat);

    cursor.insertText(
        "2. Syntax Analysis\n"
        "   - Validates structural correctness of the code and simple statements.\n"
        "   - Implemented parsers include:\n"
        "       - Delimiter Parser – checks for nested {}, (), []\n"
        "       - Assignment Parser – validates assignment statements and basic expressions\n"
        "       - Operation/Expression Parser – partially implemented for arithmetic expressions\n"
        "   - Uses a Pushdown Automaton (PDA) to simulate parsing and display the stack operations in real-time.\n\n", normalFormat);

    // GUI Features
    cursor.insertText("GUI Features:\n", subtitleFormat);
    cursor.insertText(
        "- Two main tabs: Token Table on the first tab, Parser Tabs on the second tab.\n"
        "- Each parser has its own simulator log and validation output.\n"
        "- Step-by-step visualization helps users trace how tokens are recognized and how parsing occurs.\n\n", normalFormat);
        
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(content);
}
