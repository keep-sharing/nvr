一、交叉编译
hi3536平台：
root@IT-PC-135:/home/user1/hong/smdd/arm_for_snmp/net-snmp-5.7.2.1#

./configure --prefix=/home/user1/hong/smdd/arm_for_snmp/hi3536 --build=i386-linux --host=arm-linux --with-ndianness=little --disable-manuals --with-mib-modules='ucd-snmp/diskio ip-mib/ipv4InterfaceTable ip-mib/ipv6InterfaceTable'  --enable-as-needed --disable-embedded-perl --without-perl-modules --disable-snmptrapd-subagent --disable-applications --disable-scripts --with-default-snmp-version="3" --with-sys-contact="amos@milesight.com" --with-sys-location="china" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp"  --with-cc=arm-hisiv300-linux-gcc --with-ar=arm-hisiv300-linux-ar --enable-ipv6

make clean
make LDFLAGS="-static" && make install
arm-hisiv300-linux-strip /home/user1/hong/smdd/arm_for_snmp/hi3536/sbin/snmpd



hi3798平台
root@IT-PC-135:/home/user1/hong/smdd/arm_for_snmp/net-snmp-5.7.2.1#

./configure --prefix=/home/user1/hong/smdd/arm_for_snmp/hi3798 --build=i386-linux --host=arm-linux --with-ndianness=little --disable-manuals --with-mib-modules='ucd-snmp/diskio ip-mib/ipv4InterfaceTable ip-mib/ipv6InterfaceTable'  --enable-as-needed --disable-embedded-perl --without-perl-modules --disable-snmptrapd-subagent --disable-applications --disable-scripts --with-default-snmp-version="3" --with-sys-contact="amos@milesight.com" --with-sys-location="china" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp"  --with-cc=arm-histbv310-linux-gcc --with-ar=arm-histbv310-linux-ar --enable-ipv6

make clean
make LDFLAGS="-static" && make install
arm-histbv310-linux-strip /home/user1/hong/smdd/arm_for_snmp/hi3798/sbin/snmpd


二、修改net-snmp-create-v3-user脚本
snmp v3 协议下选择no Auth,no Priv只需要输入Read/Write Security Name
生成的脚本默认都需要输入Authentication Password和Private Key Password
默认配置文件snmpd.conf



三、在NVR上执行
拷贝可执行文件snmpd到对应路径
拷贝snmpd.conf到对应路径
拷贝net-snmp-create-v3-user到对应路径

scp /home/user1/hong/smdd/arm_for_snmp/snmpd.conf /home/user1/hong/smdd/branches-x.8.0.3/targets/hi3536/nfsroot/etc/

scp /home/user1/hong/smdd/arm_for_snmp/hi3536/sbin/snmpd /home/user1/hong/smdd/branches-x.8.0.3/targets/hi3536/app/bin/
scp /home/user1/hong/smdd/arm_for_snmp/hi3536/bin/net-snmp-create-v3-user /home/user1/hong/smdd/branches-x.8.0.3/targets/hi3536/app/bin/

scp /home/user1/hong/smdd/arm_for_snmp/hi3798/sbin/snmpd /home/user1/hong/smdd/branches-x.8.0.3/targets/hi3798/app/bin/
scp /home/user1/hong/smdd/arm_for_snmp/hi3798/bin/net-snmp-create-v3-user /home/user1/hong/smdd/branches-x.8.0.3/targets/hi3798/app/bin/

运行
snmpd udp:161,udp6:161 -c /tmp/snmpd.conf

四、测试
在linux上执行命令
snmpwalk -v 2c -c readonly_v12c 192.168.9.51
snmpwalk -v 2c -c readonly_v12c udp6:[2001:f80:754::152]:161


四、snmp v3  创建命令
killall -9 snmpd
rm -f /tmp/snmpd.conf
cp -f /etc/snmpd.conf /tmp/snmpd.conf
snmpd -c /tmp/snmpd.conf
killall -2 snmpd
rm -f /var/net-snmp/snmpd.conf

sed -i 's/v3rdnoauthusername/uread1/' /tmp/snmpd.conf
sed -i 's/v3wrnoauthusername/uwrite1/' /tmp/snmpd.conf

net-snmp-create-v3-user -ro uread1
net-snmp-create-v3-user uwrite1

snmpd udp:161,udp6:161 -c /tmp/snmpd.conf
#长度和IPC一致，8-31
#端口 161 1024~65535


命令验证
snmp v1 v2c: 主机地址：192.168.9.51 2001:f80:754::152 读共同体名称：public 端口：161
获取系统基本信息.1.3.6.1.2.1.1.1.0  snmpget -v 2c -c public 192.168.9.51:161 .1.3.6.1.2.1.1.1.0
                                    snmpget -v 2c -c public udp6:[2001:f80:754::152]:161 .1.3.6.1.2.1.1.1.0
                                    如果是默认端口161，则:161可以不写。

系统运行的进程列表.1.3.6.1.2.1.25.4.2.1.2    snmpwalk -v 2c -c public 192.168.9.51 .1.3.6.1.2.1.25.4.2.1.2
                                             snmpwalk -v 2c -c public udp6:[2001:f80:754::152] .1.3.6.1.2.1.25.4.2.1.2

snmp v3 主机地址：192.168.9.51 2001:f80:754::152 读安全名称：uread 安全级别：auth,priv 认证算法：MD5 认证密码：11111111 私钥算法：DES 私钥密码：22222222
snmpget -v 3 -u uread -a MD5 -A 11111111 -l authPriv -x DES -X 22222222 192.168.9.51 .1.3.6.1.2.1.1.1.0
snmpget -v 3 -u uread -a MD5 -A 11111111 -l authPriv -x DES -X 22222222 udp6:[2001:f80:754::152] .1.3.6.1.2.1.1.1.0

snmp v3 主机地址：192.168.9.51 2001:f80:754::152 读安全名称：uread 安全级别：auth,no priv 认证算法：MD5 认证密码：11111111
snmpget -v 3 -u uread -a MD5 -A 11111111 -l authNoPriv 192.168.9.51 .1.3.6.1.2.1.1.1.0
snmpget -v 3 -u uread -a MD5 -A 11111111 -l authNoPriv udp6:[2001:f80:754::152] .1.3.6.1.2.1.1.1.0

snmp v3 主机地址：192.168.9.51 2001:f80:754::152 读安全名称：uread 安全级别：no auth,no priv
snmpget -v 3 -u uread -l noAuthNoPriv 192.168.9.51 .1.3.6.1.2.1.1.1.0
snmpget -v 3 -u uread -l noAuthNoPriv udp6:[2001:f80:754::152] .1.3.6.1.2.1.1.1.0


set命令:
设置机器名：.1.3.6.1.2.1.1.5.0
snmp v1 v2c：主机地址：192.168.9.51 2001:f80:754::152 写共同体名称：private 端口：161
snmpset -v 2c -c private 192.168.9.51:161 .1.3.6.1.2.1.1.5.0 s NVR
snmpset -v 2c -c private udp6:[2001:f80:754::152]:161 .1.3.6.1.2.1.1.5.0 s NVR

snmp v3 主机地址：192.168.9.51 2001:f80:754::152 写安全名称：uwrite 安全级别：auth,priv 认证算法：MD5 认证密码：11111111 私钥算法：DES 私钥密码：22222222
snmpset -v 3 -u uwrite -a MD5 -A 11111111 -l authPriv -x DES -X 22222222 192.168.9.51 .1.3.6.1.2.1.1.5.0 s NVR
snmpset -v 3 -u uwrite -a MD5 -A 11111111 -l authPriv -x DES -X 22222222 udp6:[2001:f80:754::152] .1.3.6.1.2.1.1.5.0 s NVR