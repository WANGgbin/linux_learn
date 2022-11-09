tar 用来压缩或者解压缩文件，也常被用来备份文件。

常用选项：
- -c 

    创建文件

- -x

    解压缩
- -z

    压缩格式： gzip

- -p

    保留(preserve)备份数据的各种属性

- -f

    后跟压缩文件名。压缩/解压缩的时候都需要通过 -f 指定压缩文件名

- -g

    可以选择增量压缩，即只压缩自某个 base 以来更改的文件。在 tar 中，这个 base 通过一个文件来生成。当该文件不存在的时候， tar 会进行全量备份并创建该 base 文件。-g 就是用来指定该 base 文件。
    > 目录还是会被记录下来，只有目录内的就文件不会备份！

- -t

    列举压缩文件的列表

- -C

    tar 默认解压缩到当前目录，可以通过 -C 选项指定解压缩目录

- --exclude
    有时候压缩某个目录，要排除一些文件，则可以通过该选项指定

- --exclude-from

    有时候要排除的文件比较多，可以将要排除的文件写到某个文件中。然后通过 --exclude-from file 的形式，将 file 内的所有文件都排除


几个例子：
- 采用 gz 格式，增量压缩目录`/home/bytedance`下的除了 `backup` 目录以外的所有文件到`/home/bytedance/backup/`

    `tar -g /home/bytedance/backup/snap -zcf /home/bytedance/backup/bytedance.tar.gz --exclude /home/bytedance/backup /home/bytedance`

- 查看压缩文件 `/home/bytedance/backup/bytedance.tar.gz` 都包含哪些文件
    `tar -tf /home/bytedance/backup/bytedance.tar.gz`

- 解压缩上述压缩文件文件到目录`/home/bytedance`中

    `tar -zxf /home/bytedance/backup/bytedance.tar.gz -C /home/bytedance`