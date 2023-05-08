# structure

```
               -------
              | shell |
        --------------------
        |    File Utility   |
    -----------------------------------
    |     inode, indirect, directory   |
----------------------------------------------
|           Free Block Manager               |
```

# disk

| block 0 | block 1 | block 2 | ... |
| --- | --- | --- | --- |
| inode for root dir | meta info | free list start ||


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

约定folder inode的第一个node指向的directory node中第一第二个entry分别为'.' 和'..'(root directory 没有'..')

# indirect block

```
Attribute (4 bytes total)
|record_num |isDir     |
|2 bytes ---|1 byte ---|
---------------
Block Number
510 block addresses, each 2 bytes
```

# 可以改进

- [x] iterator 应当返回 pointer
- [x] meta struct 设为 static const

# 剩余事项

- [ ] 先用md糊个文档
- [x] 跨平台编译
- [x] 开个branch用来提交
