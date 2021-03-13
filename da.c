#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>

#include "logger.h"
#include "elist.h"
#include "util.h"

/* Forward declarations: */
void print_usage(char *argv[]);
void fileTraversal(char *path, struct elist *list);
int file_sz_comparator(const void *a, const void *b);
int file_tm_comparator(const void *a, const void *b);

// struct to store each file information
struct item_details {
    char item[PATH_MAX];
    off_t file_sz;
    time_t file_date;
} info;

void print_usage(char *argv[]) {
fprintf(stderr, "Disk Analyzer (da): analyzes disk space usage\n");
fprintf(stderr, "Usage: %s [-ahs] [-l limit] [directory]\n\n", argv[0]);

fprintf(stderr, "If no directory is specified, the current working directory is used.\n\n");

fprintf(stderr, "Options:\n"
"    * -a              Sort the files by time of last access (descending)\n"
"    * -h              Display help/usage information\n"
"    * -l limit        Limit the output to top N files (default=unlimited)\n"
"    * -s              Sort the files by size (default, ascending)\n\n"
);
}

int main(int argc, char *argv[])
{
    /* Create a struct to hold program options and initialize it by declaring an
     * 'options' variable. Defaults:
     *      - sort by size (time=false)
     *      - limit of 0 (unlimited)
     *      - directory = '.' (current directory) */
    struct da_options {
        bool sort_by_time;
        unsigned int limit;
        char *directory;
    } options
        = { false, 0, "." };

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ahl:s")) != -1) {
        switch (c) {
            case 'a':
                options.sort_by_time = true;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            case 's':
                options.sort_by_time = false;
                break;
            case 'l': {
                /*    ^-- to declare 'endptr' here we need to enclose this case
                 *    in its own scope with curly braces { } */
                char *endptr;
                long llimit = strtol(optarg, &endptr, 10);
                if (llimit < 0 || llimit > INT_MAX || endptr == optarg) {
                    fprintf(stderr, "Invalid limit: %s\n", optarg);
                    print_usage(argv);
                    return 1;
                }
                options.limit = (int) llimit;
                break;
                }
            case '?':
                if (optopt == 'l') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (optind < argc) {
        options.directory = argv[optind];
    }

    LOGP("Done parsing arguments.\n");
    LOG("Sorting by: [%s], limit: [%u]\n",
            options.sort_by_time == true ? "time" : "size",
            options.limit);
    LOG("Directory to analyze: [%s]\n", options.directory);


    unsigned short cols = 80;
    struct winsize win_sz;
    if (ioctl(fileno(stdout), TIOCGWINSZ, &win_sz) != -1) {
        cols = win_sz.ws_col;
    }
    LOG("Display columns: %d\n", cols);

    LOG("time: %zu, off_t: %zu\n", sizeof(time_t), sizeof(off_t));

    /* TODO:
     *  - check to ensure the directory actually exists
     *  - create a new 'elist' data structure
     *  - traverse the directory and store entries in the list
     *  - sort the list (either by size or time)
     *  - print formatted list
     */
      
    DIR *directory;
    struct elist *list = elist_create(0, sizeof(struct item_details));

    if ((directory = opendir(options.directory)) == NULL) {
        perror("opendir");
        return 1;
    }

    char *pDirectory = options.directory;

    // parent Directory from the user input
    fileTraversal(pDirectory, list);
    LOG("%p\n", elist_get(list, 0));

    //struct item_details *f = elist_get(list, 0);
    //LOG("%s\n", f->item);
    //LOG("%p\n", elist_get(list, 1));
   
    /* sorting the list
     * sort by time if sort_by_time is true
     * otherwise sort by size desc */ 
    if (options.sort_by_time) {
        elist_sort(list, file_tm_comparator);
    } else {
        elist_sort(list, file_sz_comparator);
    }

    closedir(directory);
    
    struct item_details *f;
    
    /* check whether the user set limit
     * if the user set limit then print based on the specified limit
     * otherwise print all files in the specified directory */
    int numOfItems;
    if (options.limit != 0) {
        numOfItems = options.limit;
    } else {
        numOfItems = elist_size(list);
    }

    int i = 0;
    // iterating to print from dynamic array (list))
    while (i < numOfItems) {
        f = elist_get(list, i);
        char readable_sz[14];
        char last_accessed[15];
        // convert file_sz to KiB, MiB, GiB, ZiB and to string
        human_readable_size(readable_sz, 14, (double)f->file_sz, 1);
        // convert to formatted date and string
        simple_time_format(last_accessed, 15, f->file_date);

        // if the path is too long replace it with ...
        char mPath[PATH_MAX];
        if (strlen(f->item) > 51) {
            int j = strlen(f->item) - 48;
            sprintf(mPath, "...%s", f->item + j);
        } else {
            strcpy(mPath, f->item);
        }

        // print out the final result
        printf("%*s%*s%*s\n", 51, mPath, 14, readable_sz, 15, last_accessed);

        // incrementing index
        i++;
    }
   
    
    elist_destroy(list);

    return 0;
}

/**
 * Recursively getting the files from sub-directories
 * until it reach the end of sub-directories
 * Then add it to item_details struct
 * Lastly put it in the list
 *
 * @param buf path the path to be traversed
 * @param list The list to copy the struct into
 */

void fileTraversal(char *path, struct elist *list) {
    // LOG("%s\n", path);
    char newPath[PATH_MAX];
    struct dirent *de;
    DIR *d = opendir(path);

    struct stat stats;

    if (d == NULL) {
        return;
        //perror("opendir");
    }

    // iterating to read all the files from the given path
    while ((de = readdir(d)) != NULL) {
        // LOG("%s\n", de->d_name);
        // if the files inside the given path has another child directories
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) { 
            
            // concatenating the given path with the file name
            sprintf(newPath,"%s/%s", path, de->d_name);

            /* if we reach at the end of the file where it does not have another directories
             * we are storing the information to the struct and add them to the list*/
            if (opendir(newPath) == NULL) {
                
                // getting the stats from the file (time, size, etc)
                stat(newPath, &stats);
                
                // adding all these information to info struct
                strcpy(info.item, newPath);
                info.file_sz = stats.st_size;
                info.file_date = stats.st_atim.tv_sec;

                // put info struct to the elastic array list
                elist_add(list, &info);
            }

            // recursively getting all the files inside the files and so on....
            fileTraversal(newPath, list);
        }
    }
 
    closedir(d);
}

/**
 * Help function to accomodate qsort
 * This comparator is comparing two file sizes (off_t)
 *
 * @param a first element to compare
 * @param b second element to compare
 *
 * @return the comparison between the two parameters
 */

int file_sz_comparator(const void *a, const void *b) {
    const struct item_details *x = a;
    const struct item_details *y = b;

    return (int)(y->file_sz > x->file_sz);

}

/**
 * Help function to accomodate qsort.
 * This comparator is comparing time_t between 2 files (last accessed)
 *
 * @param a first element to compare
 * @param b second element to cmpare
 *
 * @return the comparison between the two parameters
 */
int file_tm_comparator(const void *a, const void *b) {
    const struct item_details *x = a;
    const struct item_details *y = b;

    return (int)(x->file_date > y->file_date);
}
