# fs_crawler

File system crawler that let you apply custom function to handle files in your system


### usage:

You can set custom function this way
``` cpp
int main()
{
    auto action = [](const char* path) -> int
    {
        std::cout << path << std::endl;
        return EXIT_SUCCESS;
    };
    std::filesystem::path start_path = "/workspace/";
    crawler(start_path, action);
    return EXIT_SUCCESS;
}
```
