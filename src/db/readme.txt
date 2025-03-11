数据库升级改动后，与原先区别如下(以hi3798为例):

1. 在\src\db路径下新增了hi3536、hi3536c、hi3536g、hi3798 共4个文件夹，用来存放各个平台非通用的数据库升级文件
因此: 原本\platform\hi3798\rootfs\opt\app\db\db-1025.txt移动到了\src\db\hi3798\db-1025.txt
因此: 后续hi3798有新增非通用的数据库升级文件，应该存放在\src\db\hi3798路径下

2. 各个平台通用的数据库升级文件依然放在\src\db路径下
因此: 后续新增各个平台通用的数据库升级文件依然存放在\src\db路径下

3. \platform\hi3798\rootfs\opt\app\db路径下现在会存放hi3798平台编译时打包的数据库升级文件压缩包db.tar.gz和db_bak.tar.gz

4. 新增加了make dbtar和make dbtar-clean两个make命令去分别执行数据库升级文件的压缩包打包/压缩包清理，这两个命令会分别在make apps gui all和make distclean时去执行，因此像往常一样编译就好了，不用去管这个

5. 数据库升级文件支持书写单行注释， 注释行以 '#' 开头

6. 原本存在的数据库升级文件命名不做改动，往后新增的数据库升级文件命名要加上版本号前缀(不需要加上-r、-a、-m等)，例如db-9.0.14-1001.txt、db-9.0.15-1001.txt、db-10.0.1-1001.txt

7. git上传代码时，不需要上传db.tar.gz和db_bak.tar.gz

