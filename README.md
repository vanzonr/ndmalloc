# ndmalloc
Dynamically allocated multi-dimensional arrays for C

##Purpose

Provide multi-dimensional arrays in C that know their shape and allow the standard repeat square bracket syntax of element access.

##Example

```c
/*
  simple_ndmalloc_example.c

  This simple example for ndmalloc creates an array of rank 2
  (i.e. a matrix) with dimensions 4x6, which starts off as all
  zeros.  The program set a few elements, and then prints the matrix. 

  Ramses van Zon, September 2016  
*/
#include <stdio.h>
#include <assert.h>
#include <ndmalloc.h>

void print_matrix(int** m)
{
    int i, j;
    for (i = 0; i < ndsize(m,0); ++i) {
        for (j = 0; j < ndsize(m,1); ++j) {
            printf("%d ", m[i][j]);
        }
        printf("\n");
    }
}

int main()
{
    int** m = ndcalloc(sizeof(int),2,4,6);
    m[0][0] = 5;
    m[3][5] = 1;
    print_matrix(m);
    ndfree(m);
}
```

To compile this (provided ndmalloc was installed already):

```sh
cc simple_ndmalloc_example.c -o simple_ndmalloc -lndmalloc
```

##Installation (Linux)

First, compile the source:
```bash
make
```

This compiles the library. Type ```make all``` to get a number of examples compiled as well.

Next, to install the library and header file in the ```/usr``` tree, type:

```bash
sudo make install
```

If you do not have root access, or you want to install in a non-standard location, you can use:

```bash
make install PREFIX=[BASEDIR]
```

with ```[BASEDIR]``` replaced with a directory of your choice. Inside ```[BASEDIR]```, files will be installed in two subdirectories, ```include``` and ```lib```.
Finally, if you use the ```PREFIX``` option, make sure these directories are added to the ```CFLAGS``` and ```LD_LIBRARY_PATH``` environment variables, respectively.

