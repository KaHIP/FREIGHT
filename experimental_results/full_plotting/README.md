Requirements
---------

The scripts contained in this folder use sqlite3. Before running the command below, download sqlite3 from https://www.sqlite.org/2022/sqlite-autoconf-3380300.tar.gz and compile it on your machine. After the compilation, replace the file sqlite/sqlite3 contained in this folder by the sqlite3 executable file you generated.

Generating Charts and Table
---------

You can plot the full charts depicted in the paper by simply running the following command

```console
./plot.sh
```

You can clean the plots by running the following command

```console
./clean_plots.sh
```

All plots will be located in plots/
