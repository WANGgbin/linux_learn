 # 问题
 - 是什么？
  sed 是 `stream editor` 的简称。可以从标准输入或者文件获取内容，处理后，输出到标准输出或者指定文件。
 - 可以用来做什么？
   - 行的操作
    sed 用于文本添加,插入,删除,替换
    sed [option] '[n, [n]]function'
    常见操作:
    a 添加一行至目标行下面 eg: sed '1,2a "something"'
    i 添加一行至目标行上面 eg: sed '1,2i "something"'
    c 替换目标行 eg: sed '1,2c "something"'
    d 删除目标行 eg: sed '1,2d'
    p 打印目标行,跟 -n 选项一起使用
    s 替换, s/source_str/target_str/g
 注意:
    除了使用数字来指定行外，我们还可以使用正则表达式来指定行。eg: `sed '/.*/d'` 删除所有内容。注意正则表达式有两种指定方式：`/regex/`，`\cregexc`(c可以是任何字符)。使用第二种方式，描述前面的例子`sed '\.\.*.d'`
   - 替代 head/tail，展示指定范围的行
    结合 `-n` 和 操作 `p`，来展示特定的行。eg: `sed -n '10,20p'`。展示第10 - 第20 行。
   - 文本替换
    可以使用 `s/old/new/g` 的方式来替换指定内容。**在 sed 中，我们可以使用任何符号(除了\\和\n)来替换\\作为分隔符， 对于内容中出现的分隔符，需要使用\\转义**。比如，我们也可以使用`s,old,new,g`。可以参考[using different delimeters in sed](https://stackoverflow.com/questions/5864146/using-different-delimiters-in-sed-commands-and-range-addresses)
   - 文件修改
    结合 `-i`选项，可以直接修改文件内容。eg:
    `sed -i 's/handle/handler/g' file`
    把 file 中的所有 handle 替换为 handler。
 - 需要注意什么？
 1. 最后一行使用那个$表示