#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>
#include <sstream>
#include <random>
#include <cstdlib>
#include <cstring>
#include <set>

void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " <filename> [options]" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  --seed <value>        Set random seed" << std::endl;
    std::cerr << "  --sample <n>          Sample n cubes from the formula" << std::endl;
    std::cerr << "  --as-cnf <i>          Output as CNF with i-th cube as unit clauses" << std::endl;
    std::cerr << "  --as-cnf-random       Output as CNF with random cube as unit clauses" << std::endl;
}

std::vector<int> parse_cube_literals(const std::string& cube_line) {
    std::vector<int> literals;
    std::istringstream iss(cube_line.substr(1)); // Skip 'a'
    int literal;
    while (iss >> literal && literal != 0) {
        literals.push_back(literal);
    }
    return literals;
}

int count_variables(const std::vector<std::string>& content, const std::vector<std::string>& cubes) {
    std::set<int> variables;
    
    // Count variables from original content
    for (const auto& line : content) {
        if (line.empty() || line[0] == 'c' || line[0] == 'p') continue;
        std::istringstream iss(line);
        int literal;
        while (iss >> literal && literal != 0) {
            variables.insert(abs(literal));
        }
    }
    
    // Count variables from cubes
    for (const auto& cube : cubes) {
        auto literals = parse_cube_literals(cube);
        for (int lit : literals) {
            variables.insert(abs(lit));
        }
    }
    
    return variables.empty() ? 0 : *variables.rbegin();
}

int count_clauses(const std::vector<std::string>& content) {
    int clauses = 0;
    for (const auto& line : content) {
        if (line.empty() || line[0] == 'c' || line[0] == 'p' || line[0] == 'a') continue;
        clauses++;
    }
    return clauses;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::random_device rd;
    std::mt19937 gen {rd()};

    std::string filename = argv[1];
    unsigned int seed_value = 0;
    bool seed_set = false;
    int sample_count = -1;
    int as_cnf_index = -1;
    bool as_cnf_random = false;

    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--seed" && i + 1 < argc) {
            seed_value = std::stoul(argv[++i]);
            seed_set = true;
        } else if (arg == "--sample" && i + 1 < argc) {
            sample_count = std::stoi(argv[++i]);
        } else if (arg == "--as-cnf" && i + 1 < argc) {
            as_cnf_index = std::stoi(argv[++i]);
        } else if (arg == "--as-cnf-random") {
            as_cnf_random = true;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    if (seed_set) {
        gen.seed(seed_value);
    }

    std::vector<std::string> cubes;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }
    
    std::string line;
    std::vector<std::string> content;
    while (std::getline(file, line)) {
        content.push_back(line);
        if (line.empty()) continue;
        if (line[0] == 'a') {
            cubes.push_back(line);
        }
    }
    file.close();

    // Handle --sample option
    if (sample_count >= 0) {
        std::ranges::shuffle(cubes, gen);
        int actual_sample = std::min(sample_count, static_cast<int>(cubes.size()));
        cubes.resize(actual_sample);
        
        // Print non-cube content
        for (const auto& content_line : content) {
            if (content_line.empty() || content_line[0] != 'a') {
                std::cout << content_line << std::endl;
            }
        }
        
        // Print sampled cubes
        for (const auto& cube : cubes) {
            std::cout << cube << std::endl;
        }
        return 0;
    }

    // Handle --as-cnf options
    if (as_cnf_index >= 0 || as_cnf_random) {
        if (cubes.empty()) {
            std::cerr << "No cubes found in file" << std::endl;
            return 1;
        }

        int cube_index;
        if (as_cnf_random) {
            std::uniform_int_distribution<> dis(0, cubes.size() - 1);
            cube_index = dis(gen);
        } else {
            cube_index = as_cnf_index - 1; // Convert to 0-based indexing
            if (cube_index < 0 || cube_index >= static_cast<int>(cubes.size())) {
                std::cerr << "Cube index out of range: " << (as_cnf_index) << std::endl;
                return 1;
            }
        }

        // Count variables and clauses
        int nvars = count_variables(content, cubes);
        int nclauses = count_clauses(content);
        auto cube_literals = parse_cube_literals(cubes[cube_index]);
        nclauses += cube_literals.size(); // Add unit clauses from cube

        // Print CNF header
        std::cout << "p cnf " << nvars << " " << nclauses << std::endl;

        // Print original clauses (skip cubes and header lines)
        for (const auto& content_line : content) {
            if (content_line.empty() || content_line[0] == 'c' || 
                content_line[0] == 'p' || content_line[0] == 'a') continue;
            std::cout << content_line << std::endl;
        }

        // Print cube literals as unit clauses
        for (int literal : cube_literals) {
            std::cout << literal << " 0" << std::endl;
        }
        return 0;
    }

    // Default behavior: shuffle cubes
    for (const auto& content_line : content) {
        if (content_line.empty() || content_line[0] != 'a') {
            std::cout << content_line << std::endl;
        }
    }
    
    std::ranges::shuffle(cubes, gen);
    for (const auto& cube : cubes) {
        std::cout << cube << std::endl;
    }

    return 0;
}