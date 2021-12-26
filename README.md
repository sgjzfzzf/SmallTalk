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
2. 可使用C/C++/C#/Java/Python等语言，Windows/Linux平台均可，可借助Socket类库。
3. 服务器与客户端可以是同一台电脑上的不同进程，也可以使用多台电脑/虚拟机实现。
4. 需要实现友好的用户界面。

## 项目环境

### 开发环境

本项目在Qt 5.12.11版本下开发完成，利用Qt MSVC编译工具链编译并生成可执行文件。

### 运行环境

在bin文件夹下已有在Windows10操作系统下编译生成的可执行文件，对应操作系统用户可直接下载运行。若是其他操作系统用户，可下载源码并本地利用Qt编译运行。

### 程序文件列表

源代码组成如下

```
source:.
│  .gitignore
│  README.md
│
├─SmallTalkClient
│      .gitignore
│      connectdialog.cpp
│      connectdialog.h
│      liveuserdialog.cpp
│      liveuserdialog.h
│      main.cpp
│      smalltalkclient.cpp
│      smalltalkclient.h
│      SmallTalkClient.pro
│      SmallTalkClient.pro.user
│      smalltalkclient.ui
│
└─SmallTalkServer
        .gitignore
        main.cpp
        smalltalkserver.cpp
        smalltalkserver.h
        SmallTalkServer.pro
        SmallTalkServer.pro.user
        smalltalkserver.ui
```

## 项目介绍

该项目利用Qt平台实现了一个简易的聊天室，包含群组聊天、发送文件、发送表情、实时查询聊天室用户等功能。

### 工作模型

该项目利用了典型的Client-Server工作模型支持运行。运行时需要一台主机运行服务端，其余用户根据服务端的ip地址和端口号进行连接。完成连接后聊天室内的用户可以通过客户端分享信息和文件等。

### 工作演示

#### 启动服务端和客户端

![](img\server1.jpg)

打开服务端，输入端口号8000（可自由选择空闲的端口号），点击创建即可成功运行一个聊天室。

![](img\client1.jpg)

![](img\client2.jpg)

打开两个客户端，输入服务器地址127.0.0.1（或者其他服务器地址），端口号8000，选择一个用户名，点击连接到服务器即可成功连接。此时服务器会进行有用户进入的提示。

![](img\server2.jpg)

提示如上，标志着两名用户成功进入聊天室。

#### 发送消息

打开Alice客户端，输入任意信息并点击发送按钮

![](img\chat1.jpg)

![](img\chat2.jpg)

发送方、接收方、服务器都成功收到了发送出的消息

#### 发送文件

打开Alice客户端，选择发送文件并随意选择一张图片

![](img\sendfile1.jpg)

Bob客户端成功收取到文件，界面提示如下

![](img\sendfile2.jpg)

进入客户端工作目录，打开doc文件夹

![](img\sendfile3.jpg)

观察到文件成功出现在其中。

值得提醒的是为了节约用户的存储空间，该文件夹默认为在关闭程序后会被自动模去，因此请及时保存需要的文件否则无法追回。

#### 发送本地表情

同时，该聊天室也支持用户发送简单的本地表情，目前支持的图片格式有png，jpg，jpeg和gif。项目针对gif进行了专门的设计，允许实时动态播放gif动画。

打开Alice客户端，选择发送本地表情

![](img\sendimg1.jpg)

选择一张图片进行发送，在Bob客户端收到对应的消息

![](img\sendimg2.jpg)

同时服务端也成功接收

![](img\sendimg3.jpg)

由于gif无法通过截图展示，故此处不进行单独演示。

#### 实时用户查询

该项目也支持实时查询聊天室在线用户。

打开Alice客户端连接下的用户列表

![](img\userlist1.jpg)

收到如下讯息

![](img\userlist2.jpg)

当前聊天室所有在线用户均被展示出来。

演示部分到此结束。

### 工作原理

#### 数据结构

本次开发主要运用的数据结构大都直接利用或继承Qt的内置数据结构，并利用Qt的信号槽机制实现了不同功能组件之间的相互互动。

##### 客户端

客户端的主要结构为自定义的SamllTalkClient类，它继承自Qt内置类QDialog，结构如下

```c++
class SmallTalkClient : public QDialog
{
    Q_OBJECT

public:
    SmallTalkClient(QWidget *parent = nullptr);
    ~SmallTalkClient();

public slots:
    void connectActionTriggered();
    void disconnectActionTriggered();
    void checkUserActionTriggered();
    void connectToServer(QString, int, QString);
    void disconnectToServer();
    void updateClient();
    void sendFile();
    void sendImg();
    void sendDataToServer();

private:
    const QString FLAG_RECEIVE = "Receive json.";
    Ui::SmallTalkClient *ui;
    QString userName;
    QTcpSocket *clientSocket;
    QMenuBar *menuBar;
    QMenu *connectMenu;
    QMenu *fileMenu;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *usersListAction;
    QAction *fileSendAction;
    QAction *imgSendAction;
    QListWidget *contentListWidget;
    QTextEdit *contentEdit;
    QPushButton *contentSendBtn;
    QGridLayout *mainLayout;
};
```

在定义的私有成员中，大部分是用于ui设计的控件成员，其余FLAG_RECEIVE为一个固定回复报文，具体作用后文将会提及，userName用于存储用户的用户名，clientSocket为连接到服务器的套接字。

##### 服务端

服务端的主要数据结构为自定义的SmallTalkServer类，它继承自Qt内置类QDialog，结构如下

```c++
class SmallTalkServer : public QDialog
{
    Q_OBJECT

public:
    SmallTalkServer(QWidget *parent = nullptr);
    ~SmallTalkServer();
    void updateClientsText(QByteArray);
    void updateClientsFile(QByteArray, QByteArray);

public slots:
    void createOrDestoryRoomServer();
    void handleNewConnection();
    void handleNewData();
    void handleDisconnection();

private:
    const QString FLAG_RECEIVE = "Receive json.";
    Ui::SmallTalkServer *ui;
    int port;
    QTcpServer *serverSocket;
    QMap<QString, QTcpSocket*> clientSockets;
    QLabel *contentLabel;
    QListWidget *contentListWidget;
    QLabel *portLabel;
    QLineEdit *portEdit;
    QPushButton *portConfirmBtn;
    QGridLayout *mainLayout;
};
```

在定义的私有成员中，大部分是用于ui设计的控件成员，其余FLAG_RECEIVE为一个固定回复报文，具体作用后文将会提及，而port用于存储该服务端打开的端口号，serverSocket保存该服务端的监听套接字。

值得一提的是clientSockets，这个数据结构帮助组织了服务端的基本功能。这个数据结构利用了Qt内置的QMap，实现了用于存储用户名的字符串和对应的套接字的对应，便于在后续的实现中进行相关套接字的查找。

##### 数据传输

数据传输依赖于json文件格式，大致如下

```json
{
    "userName":"UserName", 
    "contentType":"text/file/img", 
    "content":"The data of transmition or related information."
}
```

其中content可能为一个json object，包含了与文件相关的具体信息如文件名、大小等。

## 问题与解决

在开发过程中作者也遇到了许多问题，在这里对途中的问题和部分解决方案进行记录。

### 问题一

首先是初次使用Qt，Qt库内置丰富的控件和布局选项，但是也带来了我不熟悉这些类导致很多设想的界面工具无法正常使用的情况。

对于这样问题的解决方法主要在于大量的阅读Qt相关的教程文档，特别是CSDN上的很多分享贴为我掌握Qt提供了很大的帮助。

### 问题二

其次出现的一个问题是初期设计的不到位导致后期的修改出现很大的麻烦。一开始我的想法是依次完成作业的每一项要求，没有在开始进行代码编写之前确立好整个项目的工作模式，这在我依次完成作业要求的时候造成了非常大的麻烦。

首先比如整体的布局需要随着项目的进展逐渐向其中添加服务于不同功能的控件，但是由于缺乏前期的规划导致初次完成的外观布局十分混乱，界面上分布着大小各种的输入框和按钮，看上去不美观，于是后面我花费了大量的时间用于重新设计整理整个项目的外观布局，这本来是可以通过良好的前期设计一次开发避免的。

其次比如文件传输的设计。在开发文本传输功能的时候我直接利用字符串进行文本的分享，在这种情况下项目正常工作。但是在开发文件传输的时候由于文件过大受到TCP协议的限制，一个报文最多只能有65535字节的大小，所以过大的文件会被自动分片。如果直接发送文件内容的话，会导致接收方无法及时确认报文的接收因为其缺少确认报文完整性的手段。于是在文件数据报文发送之前会先发送一个json格式的报文用以告知对方文件的相关信息如文件名、文件大小等，然后接收方根据文件大小来判断自己接收的报文是否完整并且是否需要停止接收并进行下一步的处理。这样设计导致我初期文本发送等相关的内容也需要重新进行设计，并花费了我大量的时间在这上面。如果一开始能做好协议的设计这些重复工作完全可以避免。

### 问题三

在引入json格式的时候也出现了许多问题。发送文件时作者初期的构想是将文件数据直接添加到json文件中作为一部分进行传输，但是遇到了两个问题。首先是json文件本身不支持二进制数据的存储，这意味着无法直接将文件数据放入json中，后来的解决方法是base64编码，这种方式可以将二进制数据转换为对应的ASCII码，再利用字符串存储到json中，这是第一处问题。第二处问题在于如果直接放入json而不是分两步发送的话，无法提前将数据包大小通知到接收方，导致了接收方无法对接收到的json文件的完整性进行确认，这一问题的解决方法是倒回问题二中的方案，分两步分发报文，先利用一个小的json文件通知相关信息再发送相应的二进制数据。

### 问题四

在实现本地表情包的展示特别是gif的实时播放上也花费了很多的功夫，查阅了大量的文档和相关的资料找到了对应的类支持相关功能的实现。

## 未来与展望

对于目前开发的项目还存在不足之处，这在未来可以继续改进。首先是现有的程序都是工作在单线程模式下，服务端处理大规模并发的能力有待检验，并且单一功能的崩坏很有可能导致整体程序的崩溃。其次是UI界面仍然有待优化和改进的空间。

## 体会与建议

我认为这次大作业是一次很好的经历，成功地将课堂上学习到的有关计算机网络的知识运用在了实际的项目中，对我进一步加深对有关知识的理解发挥了很大的作用。比如文件传输时的分片机制让我认识到一开始的实现失败的原因并为我之后重写这一部分的内容提供了相关的指导。

在项目开发的过程中我也体会到了在实际的工程项目中良好的前期规划的工程管理对于项目开发带来的巨大帮助，添加注释、及时形成文档、合理使用git进行版本管理等都是需要在未来养成的良好习惯，这样宝贵的经验对于我未来的开发工作裨益良多。

最后是借此机会学习了Qt库，了解了一般桌面应用开发和前端设计的逻辑和方法，特别是Qt的信号槽机制对我触动很深刻，改变了我以往对于C++和事件驱动型编程的认知。阅读大量的文档、教程和经验贴也很大地提升了我相关的学习能力。

对于大作业的建议，我认为老师在进行大作业任务布置时可以对每项作业的核心思路和比较常用的工具进行一些简单的介绍，这样在我们进行相关资料的搜索和学习时能够指明简单的方向，而不是在网上搜索到很多无关的信息如同一个无头苍蝇一样乱撞。

## 附录

GitHub地址：https://github.com/sgjzfzzf/SmallTalk

