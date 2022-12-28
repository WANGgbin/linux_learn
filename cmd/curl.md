# 疑问
- 如何设置 ip 地址和端口
- 如何设置方法
- 如何设置 path
- 如何设置 header
- 响应的展示结果是什么样的
  
# 详述
curl 是常用的命令行数据，用来请求 web 服务器。curl 就是 client 的 url 工具的意思。
- 添加 header<br>
    使用 `-H` 添加 header。eg:
    ```sh
        curl -H 'Content-Type: application/json' -H 'Accept-Language: en-US' https://www.baidu.com
    ```
- 查看响应头<br>
    使用 `-i`，将响应头和 body 一起输出。如果只想查看响应头，可以使用 `-I`。
- 重定向<br>
    使用 `-L` 会让 http 请求自动跟随服务器的重定向。
- 将 resp 保存到文件<br>
    使用 `-o`，将 resp 保存到文件，等同于 `wget`。
    ```sh
        curl -o file https://www.baidu.com
    ```
- 指定用户名和密码<br>
    使用 `-u`，指定用户名和密码。eg:
    ```sh
        curl -u 'username:passwd' https://google.com/login
    ```
- 调试<br>
    通过`-v`查看通信的全过程，包括：tcp、SSL 握手、http req、http resp 等
- 指定请求方法<br>
    通过`-X`指定请求的方法
    ```sh
        curl -X POST https://www.baidu.com
    ```
- 发送 POST 包体<br>
    通过`-d`指定发送 POST 请求的数据。
    `-d`也可以读取本地文本文件的数据。
    ```sh
        curl -d '@data.txt' https://www.baidu.com
    ```
- 构造查询字符串<br>
    通过 `-G`结合`-d`构造查询字符串
    ```sh
        curl -G -d 'key1=val1' -d 'key2=val2' https://www.baidu.com
    ```
- 重试相关<br>
  - `--retry` 失败重试次数
  - `--retry-delay` 重试间隔
  - `--retry-max-time` 单次最大重试时间
- 指定 http 版本<br>
    `--http0.9`，`--http1.1`
- 指定 ip 版本<br>
    `--ipv4`，`--ipv6`
- 展示进度条<br>
    `--progress-bar`
# 参考资料
- [curl用法指南](https://www.ruanyifeng.com/blog/2019/09/curl-reference.html)
- [curl命令详解](https://www.jianshu.com/p/07c4dddae43a)