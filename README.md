# p2p_file_transfer

基于TCP开发的P2P文件传输

开发语言：c++
开发工具：QT5.14， MySQL5.7

项目分为中心服务器和客户端。客户端同时具有下载和提供下载的功能。

中心服务器：
中心服务器开启后，与数据库连接，数据库中存储用户账号密码信息和用户共享文件的相关信息如用户IP、端口和文件全路径等。

客户端：
由于是点对点网络结构，客户端同时也是提供下载文件的服务端。
客户端通过登录连接到中心服务器。可以上传或取消上传共享文件的相关信息到中心服务器，能通过文件名查询其他用户上传的文件然后下载。

客户端下载功能的实现过程：
客户端登录后即连接中心服务器，并开启自身的服务端模块。客户端A通过页面按钮选择目标下载文件和下载目录，槽函数将共享文件及相关信息的信息封装后传输给中心服务器。
然后中心服务器将目标文件的相关信息回传给客户端。客户端接收到数据后通过目标文件拥有者的IP连接到客户端B。连接建立后则两方开始传输数据和文件。
 