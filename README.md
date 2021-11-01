# CDA3103 Computer Logic and Organization Spr21

The files spimcore.c and project.c should be compiled together. Here is an example of how that may be done inanUNIX environment. 

First compile:$ gcc -o spimcore spimcore.c project.c

After compilation, to begin the simulation, you can type the following command in UNIX:$ ./spimcore filename.asc The command prompt cmd: should appear. 

The simulation works with the following commands (both lowercase and uppercase letter are accepted):

r “Register” - Display register contents

m “Memory” - Display memory contents

s “Step” – Attempt to run the instruction located at the current PC
  
c “Continue” – Attempt to run all instructions, beginning with PC
  
h “Halt” – Check to see if the simulation has halted
  
p “Print” – Prints a copy of the input file
  
g “Controls” – Display the most recent control signals
  
x “Quit” – terminate the simulation
