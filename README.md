# fs_crawler

File system crawler that let you apply custom function to handle files in your system

### usage:
- Indicate wich entry you do not want to handle, the program will find the matching  magic number found in Linux kernel headers with statvfs::f_fsid;
- Create a lambda to handle the entries; 
- Give a mounting point;

- The program will then crawl over your system's entries; 

Example:
``` cpp
int main()
{
    std::vector<std::string> unwanted_mount_points = {"/proc", "/sys", "/dev"};

    auto action = [](const char* path) -> int
    {
        std::cout << path << std::endl;
        return EXIT_SUCCESS;
    };
    std::filesystem::path start_path = "/workspace/src/dfs";
    crawler(start_path, action, unwanted_mount_points);

    return EXIT_SUCCESS;
}
```
