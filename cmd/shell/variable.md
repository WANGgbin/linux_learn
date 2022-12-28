- 可以按照特定模式截取变量的前缀或者后缀得到新的变量。
  参考:[shell command](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html)
  - 截取最长前缀
  
    ${var##[pattern]}

    ```sh
        var=/a/b/c/d
        last=${var##/*}
        echo $last # 空 
    ```
  - 截取最短前缀

    ${var#[pattern]}

    ```sh
        var=/a/b/c/d
        last=${var#/*}
        echo $last # a/b/c/d 
    ```
  - 截取最长后缀

    ${var%%[pattern]}
    ```sh
        var=/a/b/c/d
        last=${var%%/*}
        echo $last # 空 
    ```
  - 截取最短后缀

    ${var%[pattern]}
    eg:
    ```sh
        var=/a/b/c/d
        last=${var%/*}
        echo $last # /a/b/c
    ```