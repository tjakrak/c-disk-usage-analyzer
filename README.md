# Project 1

See spec here: https://www.cs.usfca.edu/~mmalensek/cs521/assignments/project-1.html

# Project 1: Elastic Array & Disk Usage Analyzer

## What this program do?
This program is taking a <i>path directory</i> as an input and it will sort and print out all the files from the directory specified by the user. Including the files' size and last access date.

## Implementation
The implementation of this project is divided into two parts. The first part consists of building a dynamic array to store unbounded number of elements in the heap. The second part is creating a recursive function to perform directory traversal and disk usage analysis.


The dynamic array will perform similarly just like ArrayList in java. It consist of functions to create a new list, adding elements, delete elements, accessing and removing elements at particular index. This array is mainly created using ```struct```, ```malloc```, ```realloc```, and ```memcpy```.

To read the directory and get disk usage analysis, we are recurisvely read directories with the help of standard library such as: ```dirent.h```, ```time.h``` and ```sys/stat.h```.

## Program options

| Commands  | Descriptions                              |
| --------- | ----------------------------------------- |
| -a        | Sort the files by time of last access (descending)    |
| -h        |Display help/usage information             |
|-l limit   | Limit the output to top N files (default=unlimited)                                     |
|-s         |Sort the files by size (default, ascending)|

## List of files

* <b>da.c :</b>  This is the main driver of the program. It takes input from the user, getting all the files and printing out the files.
* <b>elist.c :</b> Creating dynamic array similar to ArrayList in Java. Its main function in this program is to store all the directories, size and date within each array.
* <b>util.c :</b> A helper file which contains two functions:
    - <i>human_readable_size</i> : Convert the size of the file to (Byte, KiB, MiB,.., ZiB)
    - <i>simple_time_format</i> : Simplified date format to (Month DD YYYY)
* <b>Makefile</b>: Run and compile the program

## Unit testing
<b>List of tests:</b>
1.  create destroy - test create and destroy functions within elist.c file
2.  add get set - test add, get and set functions within elist.c file
3.  clear - test clear functions within elist.c file
4.  index of - test index of functions within elist.c file
5.  remove - test remove functions within elist.c file
6.  capacity - test capacity functions within elist.c file
7.  sort - test sort functions within elist.c file
8.  etc directory - test the program to print out all the files in etc directory
9.  long directories - test the program to be able to print long directories
10.  top 3 oldest - test the program to print out file directories sorted by date
11.  top 10 biggest - test the program to print out file directories sorted by the size
12.  broken malloc - check whether the whole program has broken malloc
13.  broken realloc - check whether the whole program has broken realloc
14.  static analysis - see if there are any unused variables, warnings, etc.
15.  leak check - see if there are any memories leaking from the program
16.  entire dataset - check the program to see if it is able to print out all the directories in the given dataset

<b>Syntax used to test:</b>

```make test``` - To test the whole tests
```make test run=x``` - To test specific case of x, where x is the test number
```make test run={x, y, ..., z}``` - To test few specific cases where {x, y, ...., z} is the test number
