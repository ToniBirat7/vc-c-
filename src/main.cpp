#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // Include standard filesystem library (C++17)
using namespace std;

// Namespace alias to make our code shorter and cleaner
namespace fs = std::filesystem;

// Helper function to read and print file contents line-by-line
void print_file_content(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "  Error: Could not open " << filepath << std::endl;
        return;
    }

    string line;
    int line_number = 1;
    while (getline(file, line)) {
        std::cout << "  [" << line_number << "] " << line << std::endl;
        line_number++;
    }
    std::cout << std::endl;
}

string get_file_content(const string& filepath) {

    ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "  Error: Could not open " << filepath << std::endl;
        return "";
    }

    string line;
    string content;
    int line_number = 1;

    while (getline(file, line)) {
        content += line + "\n";
        line_number++;
    }

    return content;
}

string fnv1a_hash(const string &data) {
    // 64-bit FNV-1a offset basis
    uint64_t hash = 14695981039346656037ULL;
    // 64-bit FNV-1a prime
    uint64_t fnv_prime = 1099511628211ULL;

    for (char c : data) {
        hash ^= static_cast<uint64_t>(c);
        hash *= fnv_prime;
    }

    // Convert the integer hash into a hex string (e.g. "d3b07384d113edec")
    stringstream ss;
    ss << hex << setw(16) << setfill('0') << hash;
    return ss.str();

}

int main() {
    std::string target_dir = "sandbox";

    // Check if the target directory exists and is indeed a directory
    if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
        std::cerr << "Error: Target directory " << target_dir << " does not exist." << std::endl;
        return 1;
    }

    std::cout << "Starting recursive traversal of: " << target_dir << "\n" << std::endl;

    // fs::recursive_directory_iterator goes through everything in the folder
    // and sub-folders recursively.
    for (const auto& entry : fs::recursive_directory_iterator(target_dir)) {
        // entry.path() gives us the path to the item.
        // fs::is_regular_file ensures we don't try to open folders as if they were files.
        if (fs::is_regular_file(entry.path())) {
            std::cout << "File: " << entry.path().string() << std::endl;

            string filepath = entry.path().string();

            print_file_content(filepath);

            // Hash and save the file content for that has hash
            string hash = fnv1a_hash(get_file_content(filepath));

            cout << "Hash for " << filepath << " is: " << hash << endl;

            // Manifest to identify hash for files

        }
    }

    return 0;
}
