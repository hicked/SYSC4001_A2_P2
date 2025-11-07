// ASSUMPTIONS PLEASE CONFIRM ENZO
// . THEIR ALLOCATE IS ONE PROCESS PER PARTITION (UNLESS CALLING FORK)... I DON'T THINK THAT'S HOW IT'S SUPPOSED TO WORK BUT

/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

// Helper to print a compact view of the wait queue (processes waiting for memory)
static std::string print_wait_queue(const std::vector<PCB>& wait_queue) {
    std::stringstream buf;
    buf << "wait queue (" << wait_queue.size() << "):\n";
    if (wait_queue.empty()) {
        buf << "(empty)\n\n";
        return buf.str();
    }
    for (const auto &p : wait_queue) {
        buf << "  - PID " << p.PID
            << ": " << p.program_name
            << " (" << p.size << "MB, partition " << p.partition_number << ")\n";
    }
    buf << "\n";
    return buf.str();
}

std::tuple<std::string, std::string, int> simulate_trace(std::vector<std::string> trace_file, int time, std::vector<std::string> vectors, std::vector<int> delays, std::vector<external_file> external_files, PCB current, std::vector<PCB> wait_queue) {

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output
    std::string system_status = "";  //!< string to accumulate the system status output
    int current_time = time;
    const int       contextSaveTime =       10;
    const int       contextResTime =        1;
    const int       ISRActivityTime =       40;

    const int       storageDeviceSpeed =    15;

    // Reserve these two numbers of system calls for
    const int       numISRFork =            2;
    const int       numISRExec =            3;
    bool            usedExec =              false;

    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        auto [activity, duration_intr, exec_program_name] = parse_trace(trace);

        if(activity == "CPU") { //As per Assignment 1
            execution += createOutputString(current_time, duration_intr, "CPU Burst");

        } else if(activity == "SYSCALL") { //As per Assignment 1
            if (duration_intr >= vectors.size()) {
                execution += createOutputString(current_time, 0, "ERROR: INVALID VECTOR TABLE INDEX: The I/O device specific drivers may be corrupted.");
            }
            else {
                std::pair<std::string, int> output = intr_boilerplate(current_time, duration_intr, contextSaveTime, vectors);
                execution += output.first;
                current_time = output.second;

                if (duration_intr < delays.size()) {
                    execution += createOutputString(current_time, ISRActivityTime, "running I/O device specific ISR (driver) due to hardware interrupt");

                    int IODelay = delays[duration_intr];

                    // Printing "." to simulate the CPU polling, or waiting for the I/O device to finish
                    int numDots = (IODelay/50 < 3) ? 3 : IODelay/50;
                    execution += createOutputString(current_time, IODelay, "IO device busy: CPU polling" + std::string(numDots, '.') + "I/O device finished, resuming");
                }
                else {
                    execution += createOutputString(current_time, ISRActivityTime, "running ISR from system call software interrupt");
                }

                // IRET (restoring)
                execution += createOutputString(current_time, contextResTime, "running IRET (restoring context)");

            }


        } else if(activity == "END_IO") {
            // Guard vector and device indices
            if (duration_intr < delays.size()) {
                auto [intr, time] = intr_boilerplate(current_time, duration_intr, contextSaveTime, vectors);
                current_time = time;
                execution += intr;

                execution += createOutputString(current_time, 1, "ending I/O for device " + std::to_string(duration_intr));
            } else {
                execution += createOutputString(current_time, 0, "ERROR: INVALID VECTOR INDEX for END_IO");
            }

            execution +=  createOutputString(current_time, contextResTime, "IRET");


        } else if(activity == "FORK") {
            // FORK, <duration>
            // prints the general stuff (save context, switch to kernel mode)
            auto [intr, time] = intr_boilerplate(current_time, numISRFork, contextSaveTime, vectors);
            execution += intr;
            current_time = time;

            //The following loop helps you do 2 things:
            // * Collect the trace of the chile (and only the child, skip parent)
            // * Get the index of where the parent is supposed to start executing from
            std::vector<std::string> child_trace;
            bool skip = true; // default to true
            bool exec_flag = false;
            int parent_index = 0;

            // start at i (where fork was called)
            for(size_t j = i; j < trace_file.size(); j++) {
                auto [_activity, _duration, _pn] = parse_trace(trace_file[j]);
                if(skip && _activity == "IF_CHILD") {
                    skip = false; // start reading for child trace file
                    continue;
                } else if(_activity == "IF_PARENT"){
                    // skip over parent stuff, it's part of the main trace
                    skip = true;
                    parent_index = j;
                    if(exec_flag) {
                        break;
                    }
                // if skip && endif basically means if it's the end of the parent
                } else if(skip && _activity == "ENDIF") {
                    skip = false;
                    continue;
                // if child calls exec
                } else if(!skip && _activity == "EXEC") {
                    skip = true;
                    // add the exec line to the trace, but ensure you set flag for exec
                    child_trace.push_back(trace_file[j]);
                    exec_flag = true;
                }

                // add to child trace file
                if(!skip) {
                    child_trace.push_back(trace_file[j]);
                }
            }
            // In the global trace file, skip to after the child and parent stuff
            i = parent_index;

            // Run the FORK ISR (duration provided in trace), copying PCB info to child
            execution += createOutputString(current_time, duration_intr, "cloning the PCB and loading child into memory (PID: " + std::to_string(current.PID+1) + ")");

            // Create child PCB (copy from parent, assign new PID, set PPID)
            // Child needs its own separate memory partition
            PCB parent = current; // Save parent before creating child
            PCB child(current.PID+1, current.PID, current.program_name, current.size, -1); // partition not assigned yet

            // Allocate new partition for child
            if (!allocate_memory(&child)) {
                execution += createOutputString(current_time, 0, "ERROR: FORK failed: no suitable partition for child process");
                execution += createOutputString(current_time, contextResTime, "running IRET (restoring context)");
                continue; // Skip this fork and continue with parent
            }

            // Call scheduler (for now just print a message)
            execution += createOutputString(current_time, 0, "scheduler called");

            // Return from ISR (IRET/restoring context)
            execution += createOutputString(current_time, contextResTime, "running IRET (restoring context)");

            // System status after FORK: child has higher priority, parent waits
            wait_queue.push_back(parent);
            current = child;

            system_status += "time: " + std::to_string(current_time) + "; current trace: FORK, " + std::to_string(duration_intr) + "\n";
            system_status += print_PCB(current, wait_queue);
            system_status += "\n";

            auto [child_execution, child_status, child_end_time] = simulate_trace(
                child_trace,
                current_time,
                vectors,
                delays,
                external_files,
                child,
                wait_queue
            );

            // Append child to overall execution log
            execution += child_execution;
            system_status += child_status;
            current_time = child_end_time;

            // After child finishes, restore parent and continue parent's execution
            execution += createOutputString(current_time, 0, "child (PID " + std::to_string(child.PID) + ") finished, resuming parent process (PID " + std::to_string(parent.PID) + ")");

            // Remove parent from wait queue and restore it as current
            if (!wait_queue.empty() && wait_queue.back().PID == parent.PID) {
                wait_queue.pop_back();
            }
            current = parent;


        } else if(activity == "EXEC") {
            // EXEC <program name>, <duration>
            usedExec = true;

            auto [intr, time] = intr_boilerplate(current_time, numISRExec, contextSaveTime, vectors);
            current_time = time;
            execution += intr;

            // Look up file size in external_files list
            int new_prog_size = -1;
            for (const external_file& file : external_files) {
                if (file.program_name == exec_program_name) {
                    new_prog_size = (int)(file.size); // could have overflow errors here but hopefully not
                    break;
                }
            }
            if (new_prog_size < 0) {
                execution += createOutputString(current_time, 0, "ERROR: EXEC failed: program not found: " + exec_program_name);
                execution += createOutputString(current_time, contextResTime, "running IRET (restoring context)");
                break;
            }

            // Free current partition
            free_memory(&current);

            // Update PCB with new program info
            current.program_name = exec_program_name;
            current.size = new_prog_size;

            execution += createOutputString(current_time, duration_intr, "program is " + std::to_string(current.size) + "Mb large");

            // Allocate new partition
            if (!allocate_memory(&current)) {
                execution += createOutputString(current_time, 0, "ERROR: EXEC failed: no suitable partition for " + exec_program_name);
                execution += createOutputString(current_time, contextResTime, "running IRET (restoring context)");
                break;
            }

            // Simulate loader: read+write each MB (1 ms per MB to match delay)
            for (int loaded = 1; loaded <= new_prog_size; ++loaded) {
                execution += createOutputString(current_time, storageDeviceSpeed, "loader: read 1Mb from disk and write to memory (" + std::to_string(loaded) + "/" + std::to_string(new_prog_size) + ")");
            }

            // Calculate remaining space for the partition
            int remaining = 0;
            if (current.partition_number >= 1) {
                remaining = (int)(memory[current.partition_number - 1].size) - new_prog_size;
            }

            execution += createOutputString(current_time, 3, "loader finished: partition " + std::to_string(current.partition_number) + " occupied: Has " + std::to_string(remaining) + "Mb free");
            execution += createOutputString(current_time, 6, "updating PCB");
            execution += createOutputString(current_time, 0, "scheduler called");
            execution += createOutputString(current_time, contextResTime, "running IRET (restoring context)");

            // System status after EXEC
            system_status += "time: " + std::to_string(current_time) + "; current trace: EXEC " + exec_program_name + ", " + std::to_string(duration_intr) + "\n";
            // Maintain visibility of any processes already waiting (e.g., original parent PID 0)
            system_status += print_PCB(current, wait_queue);
            system_status += "\n";

            // This should therefore work with recursive execs, not just once at top level
            std::ifstream exec_trace_file(exec_program_name + ".txt");

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }

            //With the exec's trace (i.e. trace of external program), run the exec recursively
            auto [exec_execution, exec_status, exec_end_time] = simulate_trace(
                exec_traces,
                current_time,
                vectors,
                delays,
                external_files,
                current,
                wait_queue
            );
            execution += exec_execution;
            system_status += exec_status;
            current_time = exec_end_time;

            break; //Why is this important? (answer in report)
            // No need to free, already did it above when we modified the PCB
            // Need break since the current trace was replaced with whatever was in exec

        } else if(activity == "IF_CHILD" || activity == "IF_PARENT" || activity == "ENDIF") {
            // Skip these control flow markers in the main trace loop
            // They are only processed inside the FORK collection logic
            continue;

        } else {
            // break early instead of continuing
            execution += createOutputString(current_time, 0, "ERROR: INVALID ACTIVITY");
            break;
        }
    }

    // When a process finishes, free its memory
    if (current.PPID == -1 && !usedExec) {
        // Parent/init process is terminating
        execution += createOutputString(current_time, 0, "process terminating (PID " + std::to_string(current.PID) + ")");
    }

    free_memory(&current); // releasing allocated memory
    return {execution, system_status, current_time};
}


int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //external_files is a C++ std::vector of the struct 'external_file'. Check the struct in
    //interrupt.hpp to know more.
    auto [vectors, delays, external_files] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    //Just a sanity check to know what files you have
    print_external_files(external_files);

    //Make initial PCB (notice how partition is not assigned yet)
    PCB current(0, -1, "init", 1, -1);
    //Update memory (partition is assigned here, you must implement this function)
    if(!allocate_memory(&current)) {
        std::cerr << "ERROR! Memory allocation failed!" << std::endl;
    }

    std::vector<PCB> wait_queue;

    //Converting the trace file into a vector of strings.
    std::vector<std::string> trace_file;
    std::string trace;
    while(std::getline(input_file, trace)) {
        trace_file.push_back(trace);
    }

    auto [execution, system_status, _] = simulate_trace(   trace_file,
                                            0,
                                            vectors,
                                            delays,
                                            external_files,
                                            current,
                                            wait_queue);

    input_file.close();

    write_output(execution, "execution.txt");
    write_output(system_status, "system_status.txt");

    return 0;
}

// Helper function
std::string createOutputString(int& totalTime, int delay, std::string msg) {
    std::string output = "";
    output += std::to_string(totalTime);
    output += ", ";
    output += std::to_string(delay);
    output += ", ";
    output += msg;
    output += "\n";
    totalTime += delay;
    return output;
}