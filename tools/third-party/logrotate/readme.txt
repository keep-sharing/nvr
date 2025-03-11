日志切割：logrotate

一、编译popt库
源码地址：https://github.com/devzero2000/POPT.git
./autogen.sh
./configure CC=arm-hisiv400-linux-gcc --host=arm-hisiv400-linux --prefix=$PWD/_install
make && make install
arm-hisiv400-linux-strip ./_install/lib/libpopt.so
scp ./_install/lib/libpopt.so* $PWD/../../../../targets/hi3536g/app/libs/


二、编译logrotate
./configure CC=arm-hisiv400-linux-gcc --host=arm-hisiv400-linux --prefix=$PWD/_install LDFLAGS="-L$PWD/../POPT/_install/lib" CPPFLAGS=-I$PWD/../POPT/_install/include
make && make install
arm-hisiv400-linux-strip ./_install/sbin/logrotate
scp ./_install/sbin/logrotate $PWD/../../../../targets/hi3536g/app/bin/

三、其他
根据需求修改examples路径下logrotate.conf文件，并拷贝到工程目录/etc/路径下
由于日志切割文件命名默认是到小时，会导致文件1小时内只能触发一次切割，需修改命名格式，logrotate.conf在dateext下面新增一行dateformat .%Y%m%d%H%M%S
在工程目录etc下创建logrotate.d文件夹，用于存放需要切割的文件配置，比如tsar.logrotate
logrotate一般是基于cron定时任务执行的，因为NVR目前不支持cron，也没有移植的必要，直接在代码里运行即可
demo:
logrotate /etc/logrotate.conf -s /tmp/logrotate.status

//-v可以输出日志
logrotate -v /etc/logrotate.conf -s /tmp/logrotate.status

~ # logrotate -f /etc/logrotate.conf -s /tmp/logrotate.status
Potentially dangerous mode on /etc/logrotate.conf: 0766
error: Ignoring /etc/logrotate.conf because it is writable by group or others.
如果遇到文件权限问题，将 chmod 644 赋予放在 logrotate.d 文件夹中的文件。
chmod -R 644 /etc/logrotate.d
