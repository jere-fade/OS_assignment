# structure

```
            -------
           | shell |
    --------------------
    |    File Utility   |
----------------------------
|    Free Block Manager     |
```

# inode

```
File Attribute (100 bytes total)
|name ----------|record_num |isdir |ctime -----|--------
|62 bytes ------|2 bytes ---|1 byte|8 bytes ---|--------
----------------
Block Number
10 direct block addresses, each 2 bytes
1 indirect block address, 2 byte
```

# directory

```
Attribute (64 bytes total)
---------------
Directory Entry (15 entries total)
|block num |name -------------------|
|2 bytes --|62 bytes ---------------|
```

# indirect block

```
Attribute (4 bytes total)
|record_num |
|2 bytes ---|
---------------
Block Number
510 block addresses, each 2 bytes
```