Student 1: Antoine Hickey
Stundet 2: Enzo Chen

Below is a brief explanation of our implementation and assumptions, as well as the answers to the questions for the assignments.


## Program Description
This programs servers to simulate the activities of a CPU. This has an emphasis on demonstrating an interrupt driven system that processes CPU activities and system calls (both software, and hardware i.e. from I/O devices). The simulation tracks timing using an `upTime` counter and generates detailed execution traces in `execution.txt`. The `trace.txt` is therefore the input file. The original Bash file was modified, and one was added: `buildnrun.sh`.

We also assumed that since there were more ISR registers in the vector table than delays in the device table, all ISRs past index 19, are exclusively software system calls (no I/O devices involved).

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