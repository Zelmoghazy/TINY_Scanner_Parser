mkdir build
gcc -c  Main.c -o build/Main.o -g -gdwarf-2 -O0 -I. -I./inc/  -Wall -Wextra -Wsign-conversion -Wconversion -MP -MD
gcc -c  src/Parser.c -o build/Parser.o -g -gdwarf-2 -O0 -I. -I./inc/  -Wall -Wextra -Wsign-conversion -Wconversion -MP -MD
gcc -c  src/Scanner.c -o build/Scanner.o -g -gdwarf-2 -O0 -I. -I./inc/  -Wall -Wextra -Wsign-conversion -Wconversion -MP -MD
gcc  build/Main.o build/Parser.o build/Scanner.o -o build/Main.exe -g -gdwarf-2 -O0 -I. -I./inc/  -Wall -Wextra -Wsign-conversion -Wconversion -MP -MD