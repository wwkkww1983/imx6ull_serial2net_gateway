# serial2net-for-topeet-imx6ull-board
基于迅为imx6ull板的串口网络网关

目前支持:\
&emsp;&emsp;TCP透传\
&emsp;&emsp;UDP透传\
&emsp;&emsp;modbus RTU与TCP互转
        
使用事项:\
&emsp;&emsp;\* 在source文件夹内执行make,在上一级目录中生成bin和cgi-bin文件夹，bin是主应用程序，cgi-bin是cgi程序\
&emsp;&emsp;\* shell文件夹存放一些运行应用程序的脚本\
&emsp;&emsp;\* SQLiteDB文件存放本地数据库,www文件夹是网页配置界面的根目录\
&emsp;&emsp;\* 拷贝除了source文件夹以外的所有文件夹到迅为板上,这些文件夹都要同级\
&emsp;&emsp;\* shell文件夹的mystart.sh是用于自启动的脚本，需自行在linux启动脚本rcs中添加运行mystart.sh的命令\
&emsp;&emsp;\* 项目没有提供4G通信模组的拨号程序，在mystart.sh中根据自己的实际情况修改拨号部分的命令\
&emsp;&emsp;\* 项目没有提供web服务器，需自己根据cgi-bin和www文件夹的实际路径配置web服务器\
&emsp;&emsp;\* ETH0网口默认IP 192.168.1.1 ETH1 192.168.1.2
