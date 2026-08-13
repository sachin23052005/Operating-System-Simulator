# 🖥️ Operating System Simulator

> **C | Operating Systems | Memory Management | Paging | Instruction Execution | Interrupt Handling**

An educational **Operating System Simulator implemented in C** as part of an Operating Systems course project.

The project simulates the execution of jobs inside a simplified operating-system environment. It demonstrates fundamental OS concepts including **memory management, program loading, instruction execution, registers, virtual-to-real address mapping, paging, page tables, job control, input/output operations, and error handling**.

The project was developed in **two phases**, progressing from a basic memory-based operating-system simulation to a more advanced paging-based implementation.

---

## 📌 Project Overview

Operating systems manage hardware resources and provide an execution environment for programs. Concepts such as memory allocation, instruction execution, address translation, input/output, and error handling are fundamental to understanding how an operating system works internally.

This project creates a simplified software simulation of these concepts.

Instead of interacting with real hardware, the simulator represents:

* Main memory
* CPU registers
* Instruction Register
* Instruction Counter
* Condition flags
* Page tables
* Virtual addresses
* Real addresses
* Program instructions
* Data input
* Output handling
* Interrupt/service requests
* Program execution limits
* Operating-system errors

The project accepts jobs through an input file, loads their instructions/data into simulated memory, executes the instructions, and produces the corresponding output or error messages.

---

# 🎯 Objectives

The primary objectives of this project are:

1. Understand the internal working of a simplified operating system.
2. Simulate memory allocation and program loading.
3. Implement instruction fetching and execution.
4. Understand CPU registers and instruction processing.
5. Implement basic operating-system service calls.
6. Implement paging and page-table based address mapping.
7. Simulate virtual-to-real address translation.
8. Handle program input and output.
9. Detect invalid instructions and operands.
10. Simulate execution time and line limits.
11. Detect invalid page access and out-of-data conditions.
12. Understand the interaction between memory management and program execution.

---

# 🏗️ Project Architecture

The project is divided into two major phases.

```text
                    OPERATING SYSTEM SIMULATOR
                              │
                 ┌────────────┴────────────┐
                 │                         │
             PHASE 1                   PHASE 2
                 │                         │
                 ▼                         ▼
       Basic OS Simulation        Paging-Based OS Simulation
                 │                         │
        ┌────────┴────────┐        ┌───────┴────────┐
        │                 │        │                │
     Memory          Instruction  Page Table    Address Mapping
     Management       Execution        │                │
        │                 │            └───────┬────────┘
        └────────┬────────┘                    │
                 │                             ▼
                 └──────────────► Program Execution
                                      │
                                      ▼
                              Output / Error Handling
```

---

# ⚙️ Phase 1 — Basic OS Simulation

The first phase implements a basic operating-system simulation using a simplified memory model.

## 🧠 Memory Model

The simulator represents memory using:

```c
char M[100][4];
```

This represents:

* **100 memory locations**
* **4 characters per memory location**

The system also maintains CPU-related components such as:

```text
Register R
Instruction Register IR
Instruction Counter IC
Condition Flag C
Service Interrupt SI
```

---

## 🧩 Registers

### General Register — `R`

A 4-character register used for loading and storing data.

### Instruction Register — `IR`

Stores the currently executing instruction.

### Instruction Counter — `IC`

Stores the address of the next instruction to be executed.

### Condition Flag — `C`

Used by the compare instruction to determine whether two values are equal.

---

# 📜 Phase 1 Instruction Set

The first phase supports the following basic instructions:

| Instruction | Operation                   |
| ----------- | --------------------------- |
| `GD`        | Get Data                    |
| `PD`        | Put Data                    |
| `LR`        | Load Register               |
| `SR`        | Store Register              |
| `CR`        | Compare Register            |
| `BT`        | Branch if Condition is True |
| `H`         | Halt                        |

These instructions provide a simplified representation of common CPU and operating-system operations.

---

## 🔹 GD — Get Data

The `GD` instruction reads data from the input and stores it in the simulated memory.

Example:

```text
GD20
```

The instruction uses the specified memory location as the destination.

---

## 🔹 PD — Put Data

The `PD` instruction writes data from simulated memory to the output file.

Example:

```text
PD20
```

---

## 🔹 LR — Load Register

Loads data from a memory location into register `R`.

```text
LR20
```

---

## 🔹 SR — Store Register

Stores the contents of register `R` into a memory location.

```text
SR20
```

---

## 🔹 CR — Compare Register

Compares the contents of register `R` with a specified memory location.

If both values are equal:

```text
C = 1
```

Otherwise:

```text
C = 0
```

---

## 🔹 BT — Branch

The branch instruction changes the instruction counter when the condition flag is set.

```text
BT09
```

This provides basic conditional program flow.

---

## 🔹 H — Halt

Terminates the execution of the current job.

```text
H
```

---

# 🔌 Master Operating System — MOS

Phase 1 uses a simplified **Master Mode / Operating System service mechanism**.

The service interrupt variable `SI` identifies the requested operating-system operation.

```text
SI = 1 → READ
SI = 2 → WRITE
SI = 3 → TERMINATE
```

The `MOS()` function handles these requests.

```text
Program Instruction
        │
        ▼
   Service Request
        │
        ▼
       SI
        │
        ▼
       MOS()
        │
   ┌────┼────┐
   ▼    ▼    ▼
 READ WRITE TERMINATE
```

This demonstrates the basic idea of how user-level instructions can request operating-system services.

---

# 📂 Job Processing

Jobs are represented using control cards such as:

```text
$AMJ
$DTA
$END
```

### `$AMJ`

Marks the beginning of a job.

### `$DTA`

Marks the beginning of the data section.

### `$END`

Marks the end of the job.

The simulator reads these control cards and performs the appropriate operations.

---

# 🧠 Phase 1 Execution Flow

```text
Input File
    │
    ▼
Read Job
    │
    ▼
$AMJ
    │
    ▼
Initialize Memory
    │
    ▼
Load Program
    │
    ▼
$DTA
    │
    ▼
Start Execution
    │
    ▼
Fetch Instruction
    │
    ▼
Decode Instruction
    │
    ▼
Execute Instruction
    │
    ├──── GD ──► READ
    │
    ├──── PD ──► WRITE
    │
    ├──── LR ──► Load Register
    │
    ├──── SR ──► Store Register
    │
    ├──── CR ──► Compare
    │
    ├──── BT ──► Branch
    │
    └──── H ───► Halt
    │
    ▼
$END
    │
    ▼
Terminate Job
```

---

# 🚀 Phase 2 — Paging-Based OS Simulation

The second phase extends the basic implementation by introducing a more sophisticated memory-management mechanism based on **paging**.

The simulated memory is expanded to:

```c
char M[4][300];
```

representing:

```text
300 memory locations
4 characters per location
```

The implementation introduces concepts including:

* Page tables
* Page table pointers
* Virtual addresses
* Real addresses
* Address mapping
* Dynamic page allocation
* Program page loading
* Data page loading
* Page validation
* Execution limits
* Program interrupt handling

---

# 🧩 Paging

Paging divides memory into fixed-size blocks called **pages** and **frames**.

The project maintains a page table that associates virtual pages with physical memory frames.

Conceptually:

```text
Virtual Address
      │
      ▼
   Page Number
      │
      ▼
  Page Table
      │
      ▼
 Frame Number
      │
      ▼
 Physical / Real Address
      │
      ▼
    Memory
```

This allows the simulated program to work with virtual addresses while the operating system maps them to actual memory locations.

---

# 🔄 MAP Function

The Phase 2 implementation contains a `MAP()` function responsible for translating a virtual address into a corresponding real address.

Conceptually:

```text
Virtual Address
       │
       ▼
     MAP()
       │
       ▼
Page Table Lookup
       │
       ▼
Real Address
```

The mapping information is stored in the simulated page table.

This demonstrates the fundamental concept of **address translation** used in paged memory-management systems.

---

# 📋 Page Table

A page table is maintained using a page-table pointer:

```text
PTR
```

The simulator searches the page table to determine the physical frame corresponding to a virtual address.

The mapping process can be represented as:

```text
Virtual Page
     │
     ▼
Page Table Entry
     │
     ▼
Physical Frame
     │
     ▼
Real Memory Location
```

---

# 🧠 Virtual and Real Addressing

Phase 2 separates the concepts of:

### Virtual Address

The address used by the simulated program.

### Real Address

The actual location in simulated physical memory.

The operating-system simulation performs the translation between the two.

For example:

```text
Program
   │
   │ Virtual Address
   ▼
Page Table
   │
   │ Frame Number
   ▼
Physical Memory
```

---

# 📜 Phase 2 Instruction Set

The second phase implements instructions including:

| Instruction | Description      |
| ----------- | ---------------- |
| `GD`        | Read data        |
| `PD`        | Write data       |
| `LR`        | Load register    |
| `SR`        | Store register   |
| `CR`        | Compare register |
| `BT`        | Branch           |
| `H`         | Halt             |

The simulator validates the instruction format and generates errors when invalid operations are encountered.

---

# 🔍 Instruction Execution

The `EXECUTE()` function is responsible for executing the current instruction.

The basic execution cycle is:

```text
Fetch
  │
  ▼
Decode
  │
  ▼
Check Instruction
  │
  ▼
Translate Address
  │
  ▼
Execute
  │
  ▼
Update Counters
  │
  ▼
Check Errors
  │
  ▼
Next Instruction
```

This represents a simplified version of the **fetch-decode-execute cycle** performed by a CPU.

---

# 🛑 Error and Interrupt Handling

One of the major improvements in Phase 2 is the introduction of error detection.

The simulator tracks different types of errors through program interrupt values.

## Program Interrupts

|  PI | Error              |
| --: | ------------------ |
| `1` | Opcode Error       |
| `2` | Operand Error      |
| `3` | Invalid Page Fault |
| `4` | Out of Data        |

---

## ⏱️ Time Limit Exceeded

The simulator maintains:

```text
TTL → Total Time Limit
TTC → Total Time Counter
```

If the execution time exceeds the permitted limit, the simulator reports:

```text
Error: TIME LIMIT EXCEEDED
```

---

## 📄 Line Limit Exceeded

The simulator also tracks:

```text
TLL → Total Line Limit
LLC → Line Limit Counter
```

If the number of output lines exceeds the permitted limit:

```text
Error: LINE LIMIT EXCEEDED
```

---

## ❌ Opcode Error

If an unsupported instruction opcode is encountered:

```text
Error: OPCODE Error
```

This prevents the simulator from executing an unknown instruction.

---

## ❌ Operand Error

If an instruction contains an invalid operand:

```text
Error: OPERAND Error
```

---

## ❌ Invalid Page Fault

If a program attempts to access an invalid or unavailable page:

```text
Error: INVALID PAGE FAULT
```

This is particularly relevant to the paging implementation.

---

## ❌ Out of Data

If the program requests input data that is unavailable:

```text
Error: OUT OF DATA
```

---

# 📊 Execution Limits

Phase 2 tracks execution statistics using variables such as:

```text
TTL
TTC
TLL
LLC
ICR
```

Where:

| Variable | Meaning                                              |
| -------- | ---------------------------------------------------- |
| `TTL`    | Total Time Limit                                     |
| `TTC`    | Total Time Counter                                   |
| `TLL`    | Total Line Limit                                     |
| `LLC`    | Line Limit Counter                                   |
| `ICR`    | Instruction Counter / instruction execution tracking |

These values are used to control execution and detect limit violations.

---

# 📥 Input Processing

The simulator reads jobs from:

```text
input.txt
```

A job contains:

```text
Job Control Information
        │
        ▼
Program Instructions
        │
        ▼
$DTA
        │
        ▼
Data
        │
        ▼
$END
```

Example structure:

```text
$AMJ...
GD20GD30...
$DTA
ABC
ABCD
IS SAME
$END...
```

The simulator identifies the different sections and processes them accordingly.

---

# 📤 Output

The program writes results and error information to:

```text
output.txt
```

Depending on the executed job, the output can contain:

* Program output
* Comparison results
* Printed data
* Error messages
* Execution information
* Job termination information

---

# 🧪 Example

A sample program may contain:

```text
GD20
GD30
GD40
GD50
LR20
CR30
BT09
PD50
H
```

The program can:

1. Read multiple pieces of data.
2. Store them in simulated memory.
3. Load values into the register.
4. Compare register and memory contents.
5. Perform conditional branching.
6. Print the appropriate result.
7. Halt execution.

---

# 🛠️ Technologies Used

### Programming Language

* **C**

### Core Concepts

* Operating Systems
* Memory Management
* Paging
* Page Tables
* Address Translation
* CPU Registers
* Instruction Execution
* Interrupt Handling
* File Handling
* Input/Output Management

### Development Environment

* GCC / C Compiler
* Windows/Linux compatible C environment

---

# 📁 Project Structure

The recommended GitHub repository structure is:

```text
OS-Paging-Simulator/
│
├── README.md
├── LICENSE
├── .gitignore
│
├── src/
│   ├── phase_1.c
│   └── phase_2.c
│
├── input/
│   └── input.txt
│
└── output/
    └── sample_output.txt
```

### File Description

| File                | Description                |
| ------------------- | -------------------------- |
| `phase_1.c`         | Basic OS simulation        |
| `phase_2.c`         | Paging-based OS simulation |
| `input.txt`         | Sample jobs and input data |
| `sample_output.txt` | Example generated output   |
| `README.md`         | Project documentation      |
| `LICENSE`           | MIT License                |

> Compiled `.exe` files are intentionally excluded from the repository because the source code is sufficient to compile the project on another system.

---

# 🚀 How to Run

## Prerequisites

Install a C compiler such as:

* GCC
* MinGW
* Clang

Verify GCC installation:

```bash
gcc --version
```

---

# ▶️ Running Phase 1

Navigate to the directory containing `phase_1.c`.

Compile:

```bash
gcc phase_1.c -o phase_1
```

Run:

```bash
./phase_1
```

On Windows:

```bash
phase_1.exe
```

The program reads:

```text
input.txt
```

and generates:

```text
output.txt
```

---

# ▶️ Running Phase 2

Compile:

```bash
gcc phase_2.c -o phase_2
```

Run:

```bash
./phase_2
```

On Windows:

```bash
phase_2.exe
```

The program reads the job definitions from:

```text
input.txt
```

and produces results in:

```text
output.txt
```

---

# 🔬 Key Concepts Demonstrated

This project provided practical implementation experience with several important Operating Systems concepts.

### 1. Memory Management

Simulated allocation and storage of programs and data in memory.

### 2. Paging

Implemented page-based memory organization and page-table mapping.

### 3. Address Translation

Converted virtual addresses into real memory addresses.

### 4. Instruction Cycle

Implemented a simplified:

```text
Fetch → Decode → Execute
```

cycle.

### 5. CPU Registers

Simulated:

* Instruction Register
* General Register
* Instruction Counter
* Condition Flag

### 6. Input/Output

Implemented file-based program input and output.

### 7. Interrupt/Error Handling

Implemented multiple simulated OS error conditions.

### 8. Resource Limits

Simulated execution-time and output-line restrictions.

---

# 💡 Learning Outcomes

Through this project, the following concepts were explored practically:

* How programs are loaded into memory.
* How instructions are fetched and executed.
* How registers participate in program execution.
* How virtual addresses can be translated into physical addresses.
* How page tables support paging.
* How an operating system can detect invalid operations.
* How input/output services can be provided to programs.
* How execution limits can be enforced.
* How different components of an operating system interact during program execution.

---

# ⚠️ Limitations

This project is an **educational simulation** and does not represent a complete real-world operating system.

Some limitations include:

* Memory is simulated using C arrays rather than actual hardware memory.
* The instruction set is intentionally simplified.
* Paging is implemented as a simulation rather than hardware-supported virtual memory.
* The simulator does not provide process scheduling or multitasking.
* There is no real CPU privilege-mode separation.
* There is no actual hardware interrupt mechanism.
* The simulator uses file-based input/output.
* Memory allocation and page selection are simplified for educational purposes.

---

# 🔮 Future Improvements

The simulator could be extended with additional operating-system concepts.

### Process Management

* Process Control Blocks
* Process creation
* Process termination
* Context switching

### CPU Scheduling

Implement algorithms such as:

* FCFS
* SJF
* Round Robin
* Priority Scheduling

### Memory Management

Add:

* Dynamic partitioning
* Segmentation
* Page replacement
* FIFO
* LRU
* Optimal page replacement

### File System

Implement a simulated:

* File allocation system
* Directory structure
* File permissions
* File operations

### Concurrency

Add:

* Threads
* Mutexes
* Semaphores
* Critical sections
* Deadlock detection

### Improved Architecture

Refactor the simulator into separate modules for:

```text
Memory Management
        │
        ├── Page Table
        │
        ├── Address Translation
        │
        └── Allocation
         
CPU Simulation
        │
        ├── Registers
        ├── Instruction Fetch
        └── Instruction Execution

I/O System
        │
        ├── Input
        └── Output

Error Handling
        │
        ├── Program Interrupts
        └── Time/Line Limits
```

---

# 📸 Sample Output

The repository contains sample input and output demonstrating the behavior of the simulator.

Example outputs include:

```text
NOT SAME

25 IS NOT GREATER

I LIKE THIS PEN OF HIS

NOT SAME
```

The test cases also demonstrate data manipulation, comparison, branching, and output operations.

---

# 🎓 Project Context

This project was developed as a **second-year Operating Systems course project** to gain practical experience implementing fundamental operating-system concepts in C.

The project focuses on understanding OS mechanisms by building a simplified execution environment rather than relying only on theoretical study.

---

# 👨‍💻 Author

**Sachin Anil Prasad**

Computer Science Engineering Student

Interests:

* Operating Systems
* Data Structures & Algorithms
* Artificial Intelligence
* Generative AI
* Cybersecurity
* Machine Learning
* Systems Programming

---

# 📜 License

This project is licensed under the **MIT License**.

See the [LICENSE](LICENSE) file for the complete license terms.

---

# ⚠️ Disclaimer

This project is intended for **educational purposes**.

It is a simplified simulation created to demonstrate Operating System concepts and should not be considered a production operating system or an implementation of an actual hardware-level OS.
