# MultiprogOS

A simplified **Operating System Simulator** that demonstrates paging, virtual memory, interrupt handling, and basic instruction execution.

The system loads jobs from **`input2.txt`**, executes them inside a simulated memory of **300 words**, and writes results to **`output2.txt`**.

----

## Project Phases

### **Phase 1**  
Basic job loading and execution:  
- Handles `GD`, `PD`, `H`  
- Tests simple input/output operations  

### **Phase 2**  
Virtual memory and paging system:  
- Implements page tables & address mapping  
- Handles page faults  
- Executes full instruction set  
- Tests **8 error codes**  
  1. Normal Termination  
  2. Out of Data  
  3. Line Limit Exceeded  
  4. Time Limit Exceeded  
  5. Operation Code Error  
  6. Operand Error  
  7. Time Limit + Operation Code Error  
  8. Time Limit + Operand Error  


