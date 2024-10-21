g++ -o histogram_plot_exe plot_histograms.cpp $(root-config --cflags --glibs) -Werror
./histogram_plot_exe
