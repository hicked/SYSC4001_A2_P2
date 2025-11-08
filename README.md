Student 1: Antoine Hickey
Stundet 2: Enzo Chen

Below is a brief explanation of our implementation and assumptions, as well as the answers to the questions for the assignments.


## Program Description
This programs servers to simulate the activities of a CPU. This has an emphasis on demonstrating an interrupt driven system that processes CPU activities and system calls (both software, and hardware i.e. from I/O devices). The simulation tracks timing using an `upTime` counter and generates detailed execution traces in `execution.txt`. The `trace.txt` is therefore the input file. The original Bash file was modified, and one was added: `buildnrun.sh`.

We also assumed that since there were more ISR registers in the vector table than delays in the device table, all ISRs past index 19, are exclusively software system calls (no I/O devices involved).

---

## Part 1: Design and Implementation of an Interrupts Simulator

### Implementation Details and Assumptions

**Core Variables**:
- `upTime`: Tracks total elapsed time in milliseconds
- `contextSaveTime`: Changes the time it takes to save the context (push whatever is in the internal registers to disk)
- `contextRestoreTime`: Changes the time it takes to restore the context from disk.
- `ISRActivityTime`: Assumed time it takes to complete all ISRs or device drivers (Note, this is seperate from the I/O device times)

**System Call Handling**: Differentiates between I/O device interrupts and regular system calls. For I/O devices, implements CPU polling simulation where the CPU waits for device completion before continuing. This is because we do not yet have a scheduler to allow for low level parallelism with the I/O devices.

**I/O Polling Simulation**: Uses dots to visually represent CPU polling time - minimum 3 dots, with additional dots based on device delay.

**Error Handling**: Validates vector table indices and trace file activities, with error reporting and possible simulation termination. Ex: If an vector table index is provided, but is out of range, it prints a message in the execution that says the drivers might be corrupted.

**Future Considerations**: Code includes notes for Assignment 2 regarding interrupt scheduling, separating ISR execution from hardware wait times, and implementing low-level parallelism. You can find more within the `interrupts.cpp` file.

---

## Part 2: Process Management & Memory Allocation

### Overview
Part 2 extends the simulator to handle **FORK** and **EXEC** system calls, implementing process creation, and memory partitioning (best-fit allocation). The simulator now manages multiple concurrent PCBs (Process Control Blocks) and produces both execution logs and system status snapshots (`system_status.txt`) showing the process table at key events.

### Key Features
- **FORK**: Clones the parent PCB to create a child process with a new PID and separate memory partition. Child processes run first (higher priority); parents wait in a queue.
- **EXEC**: Replaces the current process's memory image with a new program loaded from disk. The loader reads the program in 1Mb chunks (15ms per Mb) and reassigns partitions using best-fit.
- **Memory Management**: Six fixed partitions (40, 25, 15, 10, 8, 2 Mb) allocated via best-fit (smallest suitable partition first). Tracks occupancy and detects allocation failures.
- **Process Scheduling**: Child-first execution order; recursive simulation for nested fork/exec chains.
- **Trace Control Flow**: Supports `IF_CHILD`, `IF_PARENT`, `ENDIF` markers to separate child and parent code paths in traces.

### Memory Allocation Failures
- **FORK Failure**: If all partitions are occupied, new child cannot be allocated -> prints "ERROR: FORK failed: no suitable partition" and continues with parent.
- **EXEC Failure**: If program not found in `external_files.txt` or too large for any partition → prints error, skips loading, returns via IRET.

### Simulation Scenarios
The `output/` directory contains multiple test scenarios:
- **Simulation 1**: Basic fork with child/parent exec (program1 & program2).
- **Simulation 2**: Nested forks with varying disk speeds (15/30/45/60 ms per MB) to show loader overhead scaling.
- **Simulation 3**: Child exits immediately; parent execs and performs I/O.
- **Simulation 4**: Minimal child workload (exec + CPU burst).
- **Simulation 5**: Mutual EXEC loop (program1 ↔ program2) causing infinite recursion / segfault.
- **Simulation 6**: EXEC of oversized program (1000MB) demonstrating allocation failure.
- **Simulation 7**: Progressive partition exhaustion via nested fork/exec until 7th fork fails (only 6 partitions available).

### Build & Run
```bash
./build.sh                        # Compile the simulator
./buildnrun.sh <trace_file.txt>   # Build and run with a trace file
```

### Input Files
- `trace*.txt`: Trace files with CPU, SYSCALL, FORK, EXEC, END_IO commands.
- `vector_table.txt`: ISR addresses (26 entries).
- `device_table.txt`: I/O device delays in ms (20 entries).
- `external_files.txt`: External program names and sizes (for EXEC lookup).

### Output Files
- `execution.txt`: Timestamped log of all CPU/ISR/loader activities.
- `system_status.txt`: Multiple images of the state of the process table (PID, program, partition, size, state) after each FORK/EXEC.

---

## Implementation Notes
- **Best-Fit Allocation**: Scans partitions from smallest to largest; picks first empty partition that fits.
- **Recursion**: Both FORK (for child simulation) and EXEC (for new program traces) use recursive `simulate_trace` calls.
- **Edge Cases**: Handles allocation failure. Logs errors without crashing (except in pathological traces like simulation 5's infinite EXEC loop).
- **No Preemption**: Processes run to completion; no time-slicing or interleaving within a single trace simulation.
