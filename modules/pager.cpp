#include "modules.hpp"

Pager::Pager()
{
    filename = "";
    file_length = 0;
    num_pages = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pages[i] = NULL;
    }
}

void Pager::connect_file(const char *filenameIn)
{
    filename = filenameIn;
    std::ifstream file(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    std::streampos fsize = file.tellg();
    file.seekg(0, std::ios::end);
    file_length = file.tellg() - fsize;

    num_pages = file_length / PAGE_SIZE;
    if (file_length % PAGE_SIZE != 0)
    {
        printf("Db file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pages[i] = NULL;
    }
    file.close();
}

char *Pager::get_page(uint32_t page_num)
{
    if (page_num > TABLE_MAX_PAGES)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
               TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if (pages[page_num] == NULL)
    {
        char *page = new char[PAGE_SIZE];
        // Cache miss. Allocate memory and load from file.
        uint32_t num_pages = file_length / PAGE_SIZE;
        // We might save a partial page at the end of the file
        if (file_length % PAGE_SIZE)
        {
            num_pages += 1;
        }
        if (page_num <= num_pages)
        {
            std::ifstream file(filename, std::ios::in | std::ios::binary);
            file.seekg(page_num * PAGE_SIZE, std::ios::beg);
            file.read(page, PAGE_SIZE);
            std::streamsize bytes_read = file.gcount();
            file.close();
            if (file.rdstate() & std::ifstream::failbit)
            {
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        pages[page_num] = page;

        if (page_num >= num_pages)
        {
            num_pages = page_num + 1;
        }
    }
    return pages[page_num];
}

void Pager::flush(uint32_t page_num)
{
    if (pages[page_num] == NULL)
    {
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.seekp(page_num * PAGE_SIZE, std::ios::beg);
    if (!file.good())
    {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    file.write(pages[page_num], PAGE_SIZE);

    if ((file.rdstate() & std::ofstream::failbit) != 0)
    {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}