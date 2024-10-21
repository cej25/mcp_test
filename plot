g++ -o builder event_builder.cpp $(root-config --cflags --glibs) -Werror
./builder
