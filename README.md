# C++ Compiler Frontend Simulator - A Lexical & Syntax Analysis (An Interactive Educational Tool for Compiler Design) 

## Overview 
This **C++ Compiler Frontend Simulator - Lexical & Syntax Analyzer** is an interactive educational tool designed to demonstrate the formal language hierarchy in compiler design. It bridges the gap between abstract theory and practical application by visualizing how Python source code is transformed into structured tokens and validated for syntax in real-time.

From **Regular Expressions** to **NFAs/DFAs** and **Context-Free Grammars** via **Pushdown Automata (PDA)**, this simulator provides a comprehensive view of the front-end compilation process for a Python-like language subset.

---
## Key Features
### 1. Lexical Analysis (Scanner)
* **Tokenization:** Breaks down Python-like code into distinct tokens:
    * **Identifiers:** (`x`, `count123`, `_var`)
    * **Numbers:** (`42`, `-3.14`, `1e5`)
    * **Strings:** (`"hello"`, `'world'`)
    * **Operators:** (`+`, `*`, `>=`, `=`)
    * **Delimiters:** (`{ }`, `( )`, `[ ]`)
* **Real-Time Feedback:** Generates a live token table with precise **line** and **column numbers** for debugging.

### 2. Syntax Analysis (Parser)
* **PDA Simulation:** Implements a stack-based **Pushdown Automaton** to validate Context-Free Grammars (CFG).
* **Comprehensive Validation:** Supports full Python-like syntax structures:
    * **Assignments:** `x = 10`
    * **Expressions:** `x + y * 2`, `(a - b) / 3`
    * **Control Flow:** `if condition: ... else: ...`
    * **Function Calls:** `print(result)`
* **Live Stack Trace:** Displays a real-time log of every `PUSH` and `POP` operation.
* **Status Indicator:** Provides clear **ACCEPTED** (Green) or **REJECTED** (Red) feedback based on the parsing result.

### 3. Educational UI
  * **Modern Design:** A sleek, minimalistic dark theme featuring #16163F accents designed for optimal visual comfort.
* **Modular Interface:** Tab-based navigation separates Lexical and Syntax workflows.
* **Visual Learning:** Designed explicitly for students to "see" the internal logic of a compiler.

---

## Interface Preview

### Lexical Analysis Dashboard
<img width="100%" alt="Lexical Analysis" src="https://github.com/user-attachments/assets/3df13188-868e-4c3f-9316-b4dee759c247" />
*Visualize token generation and DFA state transitions in real-time.*

### Syntax Analysis & Stack Log
<img width="100%" alt="Syntax Analysis & Stack Log" src="https://github.com/user-attachments/assets/28daef96-d201-4edd-b95e-1123305162f5" />
*Real-time visualization of the PDA's recursive stack operations during syntax validation.*

---

## Build & Installation

### Pre-requisites
Ensure your development environment meets the following requirements:
* **OS:** Windows 10/11
* **Framework:** Qt 6.10.1 (MinGW 64-bit) — [Download Qt Online Installer](https://www.qt.io/download)
* **Build System:** CMake (v3.21+) or qmake
* **Version Control:** Git

### Step-by-Step Build Instructions

1. **Clone the repository**
   ```bash
   git clone https://github.com/itchiii16/automataproject.git
   cd Compiler-Frontend-Simulator-LEXSYN-Lexical-Syntax-Analysis
   ```
2. **Build with CMake (Recommended)**
   ```bash
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
   mingw32-make
   ```
3. **Deploy dependencies**
   ```bash
   # In Qt Command Prompt or PowerShell with Qt in PATH
   windeployqt MyQtApp.exe
   ```
4. **Run the app**
   ```bash
   ./MyQtApp.exe
   ```
---

## Future Scope
* **Dynamic Automata Generation:** Future updates aim to transition from static visualization to a **dynamic graph rendering engine**. This will allow the system to generate unique DFA/PDA diagrams on the fly based on custom user-defined Regular Expressions or Grammars.
* [cite_start]**Backend Implementation:** Planned expansion includes **Semantic Analysis**, **Intermediate Code Generation**, and **Optimization** to complete the compiler pipeline.

## Authors
* **Adanza, Aaron** 
* **Gultiano, Kathleen Grace** 
* **Jison, Remar** 
* **Laplap, Mariel** 

**Course:** Automata Theory and Formal Languages (CS311) 
