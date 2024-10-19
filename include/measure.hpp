#include <fstream>
#include <chrono>
#include <functional>

template <typename Func, typename Obj, typename... Args>
void measure(Func&& func, Obj&& obj, const std::string& filename, Args&&... args) {
    std::ofstream Csv(filename);

    for (int i = 0; i < 10000; i++) {
        // Start clock
        auto start = std::chrono::high_resolution_clock::now();

        // Call the member function with the object and arguments
        std::invoke(std::forward<Func>(func), std::forward<Obj>(obj), std::forward<Args>(args)...);

        // Stop clock, calculate duration, write to file
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        Csv << duration.count() << "\n";
    }
}