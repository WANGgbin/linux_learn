`touch` 用来更改文件的访问或者更改时间，**如果文件不存在的话，默认会创意一个时间为当前时间的文件，可以通过选项 -c 禁止创建文件**
## 选项
- `-a` 更改文件的访问时间
- `-m` 更改文件的更改时间
    > 当我们使用 `ls -al` 查看文件信息的时候，默认展示的是文件的更改时间，我们可以通过选项 -u 配合 -l 展示文件的 last access 时间而不是 last modtified 时间，比如 `ls -aul file`
- `-r` 更改文件的时间为 -r 指定的文件的时间
- `-t` 指定时间，根式为`yyyyMMddHHmm.ss`，比如202010011230.30表示2020年10月1日12点30分30秒
