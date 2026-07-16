#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

// Intution Test: Reading files recursivley

void print_file_content(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << " Error: Could not open " << filepath << endl;
        return;
    }

    string line;
    int line_number = 1;
    while (getline(file, line)) {
        cout << " [" << line_number << "] " << line << endl;
        line_number++;
    }

    cout << endl;

}

int main() {

    // Directory to read the files from
    string target_dir = "sandbox";

    // Check if it exists
    if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
        cerr << "Error: Target directory " << target_dir << " does not exists" << endl;
        return 1;
    }

    cout << "Starting recursive traversal of: " << target_dir << "\n" << endl;

    // Recursive search
    for (const auto &entry : fs::recursive_directory_iterator(target_dir)) {
        if (fs::is_regular_file(entry.path())) {
            cout << "File: " << entry.path().string() << endl;
            print_file_content(entry.path().string());
        }
    }

    return 0;
}
