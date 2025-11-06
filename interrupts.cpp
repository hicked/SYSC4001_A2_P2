// ASSUMPTIONS PLEASE CONFIRM ENZO
// 1. WHEN FORK, CHILD WILL USE THE SAME MEMORY AS PARENT (NO NEED TO REALLOCATE, JUST CREATE NEW PCB IN SAME PARTITION)
// 2. THEIR ALLOCATE IS ONE PROCESS PER PARTITION (UNLESS CALLING FORK)... I DON'T THINK THAT'S HOW IT'S SUPPOSED TO WORK BUT
// 3. WHEN EXCEC, FIND AN EMPTY PARTITION, IF NOT, ADD TO QUEUE. AGAIN, DOESN'T TRY TO FIND A HOLE IN ANOTHER PARTITION...

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
    const int       contextSavResTime =     10; // vary 10, 20, 30
    const int       ISRActivityTime =       40; // vary 40, 100, 200

    // Reserve these two numbers of system calls for
    const int       numISRFork =            2;
    const int       numISRExec =            3;


    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        auto [activity, duration_intr, exec_program_name] = parse_trace(trace);

        if(activity == "CPU") { //As per Assignment 1
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;


        } else if(activity == "SYSCALL") { //As per Assignment 1
            if (duration_intr >= vectors.size()) {
                execution += createOutputString(current_time, 1, "ERROR: INVALID VECTOR TABLE INDEX: The I/O device specific drivers may be corrupted.");
            }
            else {
                std::pair<std::string, int> output = intr_boilerplate(current_time, duration_intr, contextSavResTime, vectors);
                execution += output.first;
                current_time = output.second;

                // If there is no delay in device table, we assume that the ISR isn't for an I/O device, and is a regular system call
                // We are assuming there is no low level paralism yet, and therefore the CPU will hang until the I/O devce is finished
                // This is why, for now, we print "polling..." until the device is done before continuing
                // In the future (assignment 2), we will use an interupt schedule to allow for low level parallelism
                // Note: We decided to additionally differentiate between the software and hardware (I/O) ISRs in the output (execution.txt)
                if (duration_intr < delays.size()) {
                    execution += createOutputString(current_time, ISRActivityTime, "running I/O device specific ISR (driver) due to hardware interrupt");
                    current_time += ISRActivityTime;

                    int IODelay = delays[duration_intr];

                    // Printing "." to simulate the CPU polling, or waiting for the I/O device to finish
                    int numDots = (IODelay/50 < 3) ? 3 : IODelay/50;
                    execution += createOutputString(current_time, IODelay, "IO device busy: CPU polling" + std::string(numDots, '.') + "I/O device finished, resuming");
                    current_time += IODelay;
                }
                else {
                    execution += createOutputString(current_time, ISRActivityTime, "running ISR from system call software interrupt");
                    current_time += ISRActivityTime;
                }

                // IRET (restoring)
                execution += createOutputString(current_time, contextSavResTime, "running IRET (restoring context)");
                current_time += contextSavResTime;
            }


        } else if(activity == "END_IO") {
            // Guard vector and device indices
            if (duration_intr < delays.size()) {
                auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
                current_time = time;
                execution += intr;
                execution += createOutputString(current_time, 1, "ending I/O for device " + std::to_string(duration_intr));
                current_time += 1;
            } else {
                execution += createOutputString(current_time, 0, "ERROR: INVALID VECTOR INDEX for END_IO");
            }

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;


        } else if(activity == "FORK") {
            // FORK, <duration>
            // prints the general stuff (save context, switch to kernel mode)
            auto [intr, time] = intr_boilerplate(current_time, numISRFork, duration_intr, vectors);
            execution += intr;
            current_time = time;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your FORK output here

            // Run the FORK ISR (duration provided in trace), copying PCB info to child
            execution += createOutputString(current_time, 20, "running FORK ISR (copying parent PCB to child)");
            current_time += duration_intr;

            // Create child PCB (copy from parent, assign new PID, set PPID)
            // NO NEED TO SEARCH FOR EMPTY PARTITION SINCE IT SHARES WITH PARENT
            // I would double check the fields of the struct tho, not sure if it's right
            PCB child(current.PID+1, current.PID, current.program_name, current.size, current.partition_number);

            // Call scheduler (for now just print a message)
            execution += createOutputString(current_time, 5, "scheduler called");

            // Return from ISR (IRET/restoring context)
            execution += createOutputString(current_time, contextSavResTime, "running IRET (restoring context)");
            current_time += contextSavResTime;

            // System status after FORK: child has higher priority, parent waits
            system_status += "time: " + std::to_string(current_time) + "; current trace: FORK, " + std::to_string(duration_intr) + "\n";
            std::vector<PCB> display_wait = wait_queue;
            display_wait.push_back(current); // parent is waiting while child runs
            system_status += print_PCB(child, display_wait);
            system_status += "\n";
            // Also show the global wait queue explicitly
            system_status += print_wait_queue(wait_queue);

            ///////////////////////////////////////////////////////////////////////////////////////////

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

            // With the child's trace, run the child (recursively). Start at the current time.
            // Pass a wait queue that includes the parent so snapshots inside the child show parent waiting
            std::vector<PCB> wait_queue_for_child = wait_queue;
            auto [child_execution, child_status, child_end_time] = simulate_trace(
                child_trace,
                current_time,
                vectors,
                delays,
                external_files,
                child,
                wait_queue
            );

            // Append child's output to overall execution log
            execution += child_execution;
            // Also append child's system status snapshots
            system_status += child_status;
            // Advance time to when the child finished
            current_time = child_end_time;


        } else if(activity == "EXEC") {
            // EXEC <program name>, <duration>

            auto [intr, time] = intr_boilerplate(current_time, numISRExec, duration_intr, vectors);
            current_time = time;
            execution += intr;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your EXEC output here

            // Look up file size in external_files list
            int new_prog_size = -1;
            for (const external_file& file : external_files) {
                if (file.program_name == exec_program_name) {
                    new_prog_size = (int)(file.size); // could have overflow errors here but hopefully not
                    break;
                }
            }
            if (new_prog_size < 0) {
                execution += createOutputString(current_time, 1, "ERROR: EXEC failed: program not found: " + exec_program_name);
                execution += createOutputString(current_time, contextSavResTime, "running IRET (restoring context)");
                current_time += contextSavResTime;
                // Snapshot failure
                system_status += "time: " + std::to_string(current_time) + "; current trace: EXEC " + exec_program_name + ", " + std::to_string(duration_intr) + " (FAILED: not found)\n";
                system_status += print_PCB(current, wait_queue);
                system_status += "\n";
                system_status += print_wait_queue(wait_queue);
                break; // proceed with rest of trace
            }

            // Free current partition
            free_memory(&current);

            // Update PCB with new program info
            current.program_name = exec_program_name;
            current.size = new_prog_size;

            // Allocate new partition
            if (!allocate_memory(&current)) {
                execution += createOutputString(current_time, 1, "ERROR: EXEC failed: no suitable partition for " + exec_program_name);
                execution += createOutputString(current_time, contextSavResTime, "running IRET (restoring context)");
                current_time += contextSavResTime;
                wait_queue.push_back(current);

                // Snapshot failure to allocate as well
                system_status += "time: " + std::to_string(current_time) + "; current trace: EXEC " + exec_program_name + ", " + std::to_string(duration_intr) + " (FAILED: no partition)\n";
                system_status += print_PCB(current, wait_queue);
                system_status += "\n";
                system_status += print_wait_queue(wait_queue);

                continue;
            }

            // Simulate loader: read+write each MB (1 ms per MB to match delay)
            for (int loaded = 1; loaded <= new_prog_size; ++loaded) {
                execution += createOutputString(current_time, 1, "loader: read 1MB from disk and write to memory (" + std::to_string(loaded) + "/" + std::to_string(new_prog_size) + ")");
                current_time += 1;
            }

            int remaining = 0;
            if (current.partition_number >= 1) {
                remaining = static_cast<int>(memory[current.partition_number - 1].size) - new_prog_size;
                if (remaining < 0) remaining = 0;
            }
            execution += createOutputString(current_time, 1, "loader finished: partition " + std::to_string(current.partition_number) + " has " + std::to_string(remaining) + "MB free");
            execution += createOutputString(current_time, 5, "scheduler called");
            execution += createOutputString(current_time, contextSavResTime, "running IRET (restoring context)");
            current_time += contextSavResTime;

            // System status after EXEC
            system_status += "time: " + std::to_string(current_time) + "; current trace: EXEC " + exec_program_name + ", " + std::to_string(duration_intr) + "\n";
            system_status += print_PCB(current, wait_queue);
            system_status += "\n";
            system_status += print_wait_queue(wait_queue);

            ///////////////////////////////////////////////////////////////////////////////////////////


            std::ifstream exec_trace_file(exec_program_name + ".txt");

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }

            ///////////////////////////////////////////////////////////////////////////////////////////
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

            ///////////////////////////////////////////////////////////////////////////////////////////

            break; //Why is this important? (answer in report)
            // No need to free, already did it above when we modified the PCB
            // Need break since the current trace was replaced with whatever was in exec

        } else if(activity == "IF_CHILD" || activity == "IF_PARENT" || activity == "ENDIF") {
            // Skip these control flow markers in the main trace loop
            // They are only processed inside the FORK collection logic
            continue;

        } else {
            // break early instead of continuing
            execution += createOutputString(current_time, 1, "ERROR: INVALID ACTIVITY. ENDING SIMULATION");
            current_time += 1;
            break;
        }
    }
    free_memory(&current); // releasing allocated memory

    // see if anything in the queue
    // for (int i = 0; i<wait_queue.size(); i++) {
    //     if (allocate_memory(&wait_queue[i])) {
    //         // allocate the memory,
    //         // run the new trace associated with the process
    //         // remove from wait queue
    //     }
    // }

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

    /******************ADD YOUR VARIABLES HERE*************************/


    /******************************************************************/

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
std::string createOutputString(unsigned long totalTime, int delay, std::string msg) {
    std::string output = "";
    output += std::to_string(totalTime);
    output += ", ";
    output += std::to_string(delay);
    output += ", ";
    output += msg;
    output += "\n";
    return output;
}