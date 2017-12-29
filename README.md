
## colandreas-db-modelselector

filter [colandreas](https://github.com/Pottus/ColAndreas) db files

### compiling

```
gcc -Wall -o cdbms main.c
```

### typical use
```
cat modellist.txt | uniq | ./cdbms -iin.cadb -oout.cadb
```

where modellist.txt contains model ids on separate lines

### options

```
-i<file>    inputfile
-o<file>    outputfile
```

