

# I/O操作函数

## 创建文件描述符

pipe

dup/dup2 (用于文件描述符重定向)

GCI 服务器？



### iovec

``iovec``用于将数据进行分散的io操作，每个iovec指向一个分散的数据块，用于读取/写入固定长度的数据。

```cpp
/* Structure for scatter/gather I/O.  */
struct iovec
  {
    void *iov_base;	/* Pointer to data.  */
    size_t iov_len;	/* Length of data.  */
  };
```

``iovec``的使用方式，集中写/分散读，需要对``iov_len``进行初始化操作。

```cpp
// writev 集中写
std::vector<iovec> iovecs(5);
for(auto &it:iovecs){
    it.iov_base = new char[]{"abcd\n"};
    //iov_len 要初始化
    it.iov_len = 10;
}
writev(clientfd, iovecs.data(), static_cast<int>(iovecs.size()));
for(auto &it:iovecs){
    delete [] (char*)it.iov_base;
}

// readv 分散读
std::vector<iovec> iovecs(5);
for (auto &it : iovecs) {
    it.iov_base = new char[10];
    it.iov_len = 10;
}
readv(serverfd, iovecs.data(), static_cast<int>(iovecs.size()));
for (auto &it : iovecs) {
    printf("%s", (char *)it.iov_base);
    delete[] (char *)it.iov_base;
}
```



#### splice

``splice``用于零拷贝传输。其底层原理是通过内核的管道传输数据，无需将数据拷贝到用户态。

```cpp
//利用 splice + pipe 使数据从io缓冲区直接发送到网卡缓冲区 无需经过拷贝
int pipe_io[2];
ret = pipe(pipe_io);
assert(ret >= 0);
//bind pipe 
ret = splice(clientfd, NULL, pipe_io[1], NULL, 100, 0);
assert(ret >= 0);
ret = splice(pipe_io[0], NULL, clientfd, NULL, 100, 0);
assert(ret >= 0);
```



#### tee

``tee``用来做管道拷贝

```cpp
//create pipe
int pipe_stdout[2];
int pipe_recv[2];
ret = pipe(pipe_stdout);
assert(ret>=0);
ret = pipe(pipe_recv);
assert(ret>=0);

auto fildfd = open("log.txt", O_RDONLY|O_WRONLY|O_CREAT);
//splice recv and clientfd

splice(clientfd, NULL, pipe_recv[1], NULL, 100, 0);
tee(pipe_recv[0], pipe_stdout[1], 100, 0);
splice(pipe_stdout[0], NULL, STDOUT_FILENO, NULL, 100, 0);
splice(pipe_recv[0],NULL,fildfd,NULL,100,0);

close(fildfd);
```

