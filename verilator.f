// Strict warnings
-Wall
// Don't exit on warnings
-Wno-fatal
// Multithreading
-j 0
// Dump Trace
--trace 
// Dump readable structs
--trace-structs
// Compile C exe
-cc --exe

// all exploicit Xs are replaced by a constant at runtime
--x-assign unique
--x-initial unique

--timing
