# Linux Server 编程规范

## 日志

### 日志api

``syslog``

``openlog``

``setlogmask``

``closelog``

### 日志系统
日志系统的结构：
![alt text](image.png)

日志系统配置文件设置 ``vim /etc/rsyslog.conf``，可以控制不同级别日志文件写入到哪里。


## 用户/用户组
