# SmallTalk

这个项目是计算机网络通信大作业的记录。在这个作业中将尝试实现一个简易的网络聊天室。

## 作业要求

### 项目任务

1. 一对一聊天程序：两个用户之间实现网络数据传输。
2. 多用户聊天程序：分为服务器与客户端，服务器能够支持多个用户之间的一对一聊天，实现网络数据传输（实现功能2即不必实现功能1）。工作过程：服务器启动后，侦听指定端口，客户端使用套接字传输消息，由服务器转发至另一客户端。
3. 文件传输：实现用户之间的文件传输，不限文件类型。
4. 扩展功能：参考现有聊天程序扩展功能（例如群组聊天、使用表情、语音聊天等）。

### 项目要求

1. 一人独立完成。
2. 可使用C/C++/C#/Java/Python等语言，Windows/Linux平台均可，可 借助Socket类库。
3. 服务器与客户端可以是同一台电脑上的不同进程，也可以使用多台电 脑/虚拟机实现。
4. 需要实现友好的用户界面。

## 实现想法

### 开发环境

本次作业使用QT进行。

### 初步构想

本项目采用Tcp通信，将主要分为两部分，客户端和服务端。进行聊天通信时客户端将信息或文件传输给服务端，由服务端分发给同一聊天室内的其他客户端。

#### 服务端

每当创建一个新的聊天室时，便需要启动一个新的服务端。初步设想的服务端界面由如下控件组成

- 聊天内容窗口
- 监听端口号和确认键，确认键在连接成功后会变成断开键

#### 客户端

客户端主要由如下控件组成

- 聊天内容窗口
- 聊天内容输入，发送键
- 发送文件选择，发送键
- 服务器IP号，服务器端口号，连接键（或者断开键）
- （可能会有的）语音聊天模块

##### 文件发送

这里先记录一下文件发送功能的构想。当用户发送文件后在别的用户端会弹出一个Infomation窗口选择是否接收文件，如果是就选择文件保存的文件夹，点否就失去保存文件的机会了。

#### 数据格式

使用json格式，如下

```json
{"content_type":"text/file/sound", "content":"data of transmition"}
```

（可能会再添加一个关于返回状态的字段，视实际需求决定）