#!/bin/env bash
# sed 用于文本添加,插入,删除,替换
# sed [option] '[n, [n]]function'
# 常见操作:
# a 添加一行至目标行下面 eg: sed '1,2a "something"'
# i 添加一行至目标行上面 eg: sed '1,2i "something"'
# c 替换目标行 eg: sed '1,2c "something"'
# d 删除目标行 eg: sed '1,2d'
# p 打印目标行,跟 -n 选项一起使用
# s 替换, s/source_str/target_str/g
# 注意:
# 1. 最后一行使用那个$表示
# 2. 目标行可以通过行号指定,也可以通过正则指定 eg: sed '/^[a-z]/d'