1.����linux�ںˣ�ʹ��֧��pppoe�������������ļ�����NVRоƬ
2.����ppp-2.4.5Դ�룬�����������ļ�����NVRоƬ
3.����rp-pppoe-3.8Դ��
4.�����������ɵ����ýű��޸ĺ��޸��ļ�·����������������
5.��������



��1��
���ȵ����ں�֧��PPP������Linux �ں�Ŀ¼��ִ�� #make menuconfig
��������ں�ѡ�
Device Drivers ---> Network device support --->
<*> PPP (point-to-point protocol) support
[*]   PPP multilink support
<*> PPP support for async serial ports
<*> PPP support for sync tty ports
<*> SLIP (serial line) support
[*]   CSLIP compressed headers
�����ɺ󱣴沢�˳���ִ��# make zImage
��������Ժ����ں�Ŀ¼��arch/arm/boot Ŀ¼�±������һ�� zImage �ں��ļ����յ��������У������������ں˾�֧����PPP�ˡ�

�����ɵ�uImage����\branches-x.9.0.4\targets\hi3536\firmware�µ�uImage    mv ./uImage ./branches-x.9.0.4/targets/hi3536/firmware/
���뾵���ļ�   make apps gui all    
������õľ����ļ�����tftpboot�ļ���    cp ./targets/hi3536/firmware/MSFImage_71.9.0.4-r1 /tftpboot/
��NVR�н���ubootģʽ��Ȼ����¼  download


��2��
��ѹԴ���  tar zxvf ppp-2.4.5.tar.gz
����Դ���ļ���  cd ppp-2.4.5
1�������ļ�  ./configure --prefix=
2������  make CC=arm-hisiv300-linux-gcc  �������˽����������������δ�ҵ���ִ��   export PATH=$PATH:/opt/hisi-linux/x86-arm/arm-hisiv300-linux/target/bin��
3������Makefile�ļ���installĬ����pc��strip����Ҫɾ���ü��Ĳ��裬install ���-s��Ҫȥ����
4��make install 
5��arm-hisiv300-linux-strip 
6������pppd�ļ��������ɵ�pppd��ִ���ļ�������Ŀ��壨NVRоƬ����/opt/app/bin/�ļ�����  scp ./pppd/pppd 192.168.2.131:/opt/app/bin/
��NVR��/opt/app/bin/Ŀ¼������pppd������������˵����һ���ɹ��ˣ����ں˲�֧�ֻ�����ʾ


��3��
��ѹԴ���  tar zxvf rp-pppoe-3.8.tar.gz
����Դ���src�ļ���  cd rp-pppoe-3.8/src
�����ļ� ./configure --host=arm-linux CC=arm-hisiv300-linux-gcc   �������˱���ƽ̨�ͽ����������
�޸�makefile�ļ�����������src�º�libevent�£���gcc->arm-hisiv300-linux-gcc    ��   ar->arm-hisiv300-linux-ar
���� make all
��װ make install


��4��
��rp-pppoe-3.8/configs�µ����нű��ļ�������NVR�ϵ�/etc/ppp/Ŀ¼�£�������Ŀ¼���Ͽ�дȨ��
��/usr/sbin�²���������pppoe�ļ���pppoe��pppoe-server��pppoe-sniff��pppoe-relay��pppoe-setup�� pppoe-start��pppoe-stop��pppoe-status��pppoe-connect��������NVR�ϵ�/opt/app/bin/Ŀ¼�£����޸����е�����ļ�·����ʹ��������NVR������


��5��
ִ��pppoe-setup   �����û�������Ϣ
ִ��pppoe-start



