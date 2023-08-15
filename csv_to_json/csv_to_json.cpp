#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <Windows.h>

#include "csv_parser.h"

namespace perf
{
    // Rough measure of general program performance.

    int64_t start_time = 0;
    double perf_freq = 0.0;

    void start_counter()
    {
        LARGE_INTEGER li;

        if (!QueryPerformanceFrequency(&li))
            return;

        // Divide to get milliseconds.
        perf_freq = li.QuadPart / 1000.0;
        QueryPerformanceCounter(&li);
        start_time = li.QuadPart;
    }
    double stop_counter()
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);

        // Calculate delta.
        return (li.QuadPart - start_time) / perf_freq;
    }
}

int main()
{
    // Start timer.
    perf::start_counter();

    // Open target .csv.
    auto file = new csv_file<temperature_sensor_entry>("../temperatures.csv");

    // Extract .csv entries.
    std::vector<temperature_sensor_entry> out;
    out = file->get_entries();
 
    // Open output file.
    std::ofstream outfile("../output.json");
    if (outfile.is_open())
    {
        // Create parent group.
        outfile << file->create_json_group("Temperature");

        // Loop all data entries and convert.
        int i = 0;
        for (auto entry : out)
        {
            if (i == (out.size() - 1))
                outfile << entry.to_json_entry(true);
            else
                outfile << entry.to_json_entry();

            i++;
        }

        // Close group and file.
        outfile << file->close_json_group();
        outfile.close();
    }

    // Stop timer.
    auto delta = perf::stop_counter();

    // Print that we are done and wait for user input.
    std::cout << "[+] Done.\n";
    std::cout << "[+] Took " << delta << "ms\n";
    std::cin.get();

    return 0;
}