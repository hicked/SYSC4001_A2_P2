/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

std::tuple<std::string, std::string, int> simulate_trace(std::vector<std::string> trace_file, int time, std::vector<std::string> vectors, std::vector<int> delays, std::vector<external_file> external_files, PCB current, std::vector<PCB> wait_queue) {

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output
    std::string system_status = "";  //!< string to accumulate the system status output
    int current_time = time;
    const int       contextSavResTime =     10; // vary 10, 20, 30
    const int       ISRActivityTime =       40; // vary 40, 100, 200


    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        auto [activity, duration_intr, program_name] = parse_trace(trace);

        if(activity == "CPU") { //As per Assignment 1
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;
        } else if(activity == "SYSCALL") { //As per Assignment 1
            if (duration_intr >= vectors.size()) {
                execution += createOutputString(current_time, 0, "ERROR: INVALID VECTOR TABLE INDEX: The I/O device specific drivers may be corrupted.");
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
                current_time += ISRActivityTime;
            }
        } else if(activity == "END_IO") {
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            current_time = time;
            execution += intr;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", ENDIO ISR(ADD STEPS HERE)\n";
            current_time += delays[duration_intr];

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        } else if(activity == "FORK") {
            // prints the general stuff (save context, switch to kernel mode)
            auto [intr, time] = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr;
            current_time = time;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your FORK output here

            // print forking... or something

            ///////////////////////////////////////////////////////////////////////////////////////////

            //The following loop helps you do 2 things:
            // * Collect the trace of the chile (and only the child, skip parent)
            // * Get the index of where the parent is supposed to start executing from
    
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_flag = false;
            int parent_index = 0;

            for(size_t j = i; j < trace_file.size(); j++) {
                auto [_activity, _duration, _pn] = parse_trace(trace_file[j]);
                if(skip && _activity == "IF_CHILD") {
                    skip = false;
                    continue;
                } else if(_activity == "IF_PARENT"){
                    skip = true;
                    parent_index = j;
                    if(exec_flag) {
                        break;
                    }
                } else if(skip && _activity == "ENDIF") {
                    skip = false;
                    continue;
                } else if(!skip && _activity == "EXEC") {
                    skip = true;
                    child_trace.push_back(trace_file[j]);
                    exec_flag = true;
                }

                if(!skip) {
                    child_trace.push_back(trace_file[j]);
                }
            }
            i = parent_index;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the child's trace, run the child (HINT: think recursion)
            // I guess child can have other children so we need to call simulate trace again...?



            ///////////////////////////////////////////////////////////////////////////////////////////


        } else if(activity == "EXEC") {
            auto [intr, time] = intr_boilerplate(current_time, 3, 10, vectors);
            current_time = time;
            execution += intr;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your EXEC output here



            ///////////////////////////////////////////////////////////////////////////////////////////


            std::ifstream exec_trace_file(program_name + ".txt");

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the exec's trace (i.e. trace of external program), run the exec (HINT: think recursion)



            ///////////////////////////////////////////////////////////////////////////////////////////

            break; //Why is this important? (answer in report)

        }
        else {
            // break early instead of continuing
            execution += createOutputString(current_time, 0, "ERROR: INVALID ACTIVITY. ENDING SIMULATION");
            break;
        }
    }

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