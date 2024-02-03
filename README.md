# SimpleCPUSim

SimpleCPUSim is a speculative, multi-issue, out of order CPU simulator command line program that supports a small subset of RISC-V instructions. As input, it takes an assembly program to run and a configuration file to control several parameters of the virtual CPU. It then prints the final contents of the register file and data cache, as well as some statistics about the execution.

### Building 

1. Install required dependencies with `sudo apt install build-essential`
2. In the project's root directory, run the command `make`
3. The final executable is located at `./bin/cpu_sim`

### Running 

#### Input Assembly File

The input assembly file contains the program to be executed by the CPU simulator. It supports setting intial memory contents, 10 RISC-V instructions, labels, and comments.

An example input file is located at `input/prog.dat` (the extension is unimportant)

**Memory setting:**
* Memory is set at the beginning of the file
* In a line, set a value at a given address with the format: `address, value`
  * `address` and `value` must be decimal (base 10) integers
* Repeat this pattern as needed

**Instructions:**
* The instructions follow the lines setting the memory contents and a blank newline as spacing
* Supported instructions:
  * fld, fsd, add, addi, slt, fadd, fsub, fmul, fdiv, bne
* To provide an instruction with a label, simply write `label_name:` preceeding the instruction on the same line
  * Where `label_name` is your custom name

#### (Optional) Configuration File

The configuration file allows for the control the operating parameters of different units within the CPU, such as the number of instructions that can be fetched and issued every cycle, as well as the size of different queues, buffers, and busses. 

There are 5 total parameters that can be changed:

Parameter | Meaning
---|---
NF | Maximum number of instructions that can be fetched from the input data every cycle
NI | Size limit of the decode unit instruction queue
NW | Maximum number of instructions that can be issued to reservation stations every cycle
NR | Number of entries in the reorder buffer (ROB)
NB | Number of common data busses (CDBs)

**File format:**
```
NF 4
NI 16
NW 4
```
Place the name of each parameter followed by the number you want to set on separate lines. You can set only 1 parameter or all of them. Any parameters not found in the provided configuration file will use their default values (which can be located in an example file at `input/config.txt`)

#### Actually Running Things

After building the package, SimpleCPUSim can be run with the command:

`./bin/cpu_sim <path_to_input_file>`

Or, to run SimpleCPUSim with a configuration file, provide its path before the input file:

`./bin/cpu_sim <path_to_config_file> <path_to_input_file>`

