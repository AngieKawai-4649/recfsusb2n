# 慶安 KTV-FSUSB2N Linux用チューナーアプリケーション

ベース　recfsusb2n ver. 0.9.2

対象デバイス

VENDOR-ID   0x0511  
PRODUCT-ID  0x0029 or 0x003b

## 【改造箇所】

1.コンパイル時のワーニング除去  
２．内蔵カードリーダー使用時、カードリーダーアクセス処理を別プロセスに分離（詳細はktv_cardreaderを参照）  
３．チューナーデバイスサーチ時、マルチスレッド処理において使用していたboostライブラリを  
　　linuxシステムコールライブラリ、pthreadライブラリを使用するように修正  
４．tssplitter_lite機能を追加  
５．デバイスを直接指定して起動する機能を追加  
６．B25デコーダープログラムを抱え込んでビルドしていたのを削除し、libarib25.soをリンクするように修正  

## 【ビルド方法】  
  Makefileのコメントを参照


## 【使用方法】

$ recfsusb2n [-v] [--b25] [--dev devfile] [--wait n (1wait==100milsec)] [--sid hd,sd1,sd2,sd3,1seg,all,epg,epg1seg] channel recsec destfile

-v : ログを標準出力する  
--b25 : B25解除する  B25解除を行わないでビルドした場合は指定不可  
--dev : デバイスを指定する  
例： lsusb の結果 Bus 003 Device 002: ID 0511:0029 N'Able (DataBook) Technologies, Inc. の場合  
　ubuntuでは --dev /dev/bus/usb/003/002 と指定するが各環境のUSBデバイスパスを調べて指定すること  
　また、USBコネクタの接続する場所を変えるとデバイスパスが変わるので注意  
　無指定の場合は接続された未使用のデバイスを順に使用する
   
--wait n : チューナーを開けてストリームが安定するまでの待ち時間を1wait==100milsec単位で指定する  
--sid : tssplitter_liteでTSを分割し特定のストリームを抽出する場合に指定、複数オプションはカンマで区切る
   
B25解除を行わないでビルドした場合は指定不可
           
例： ワンセグとEPG1segを録画する場合  
　--sid 1seg,epg1seg  

注： mirakurunを導入している場合、mirakurunで指定したsidのストリームを抽出できるので  
　tssplitter_lite機能を使用する必要はない  
　channel: 地デジチャンネルを指定する  
　rectime: 録画時間を秒単位で指定する  
　destfile: TS出力ファイル名を指定する  

例: NHK東京 1segをデバイスを指定して30秒録画する  
$ ./recfsusb2n -v --b25  --dev /dev/bus/usb/003/002 --sid 1seg 27 30 aaa.ts  
注 b25解除後にtssplitterで分割しているので--b25を指定すること  

NHK東京 1segを取り除きhdを30秒録画する  
$ ./recfsusb2n -v --b25 --sid hd 27 30 bbb.ts

## 【USBデバイスの固定】  
USB機器を追加、取り外しを行うとチューナーデバイスのUSBパスが変わってしまうので  
チューナーを接続しているUSBポートの物理的な場所に対してチューナーデバイス名称を付ける  

$ lsusb で各チューナーデバイスのUSBパスを調べる  
$ udevadm でUSBポートの物理的な場所を調べる  

例 $ udevadm info -n /dev/bus/usb/001/005 -q path    
　出力値例 /devices/pci0000:00/0000:00:08.1/0000:05:00.3/usb1/1-4/1-4.1 が物理的な場所である  
　rulesファイルでDEVPATHに指定する  
　SYMLINKで指定した名称で /devにデバイスのシンボリックリンクが作成されるのでmirakurun tuners.ymlでそのデバイス名称を使用する  

ATTRを調べる例 $ udevadm info -a -p `udevadm info -q path -n /dev/bus/usb/001/005`  

## [ruleファイルの作成]  
/lib/udev/rules.d  and /etc/udev/rules.d 配下のファイルをチェックし、使用していない番号のルールファイルを/etc/udev/rules.dに作成する  
例：93-tuner.rules  
　ファイル保存後、ルールを反映  
　$ sudo udevadm control --reload  
　リブート後、デバイスを確認する  
　$ ls /dev  

## [ruleファイルの内容例]  
\# FSUSB2N  
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb_device", ATTRS{idVendor}=="0511", ATTRS{idProduct}=="0029",DEVPATH=="/devices/pci0000:00/0000:00:08.1/0000:05:00.3/usb1/1-4/1-4.3", MODE="0664", GROUP="video", SYMLINK+="FSUSB2N_1"  
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb_device", ATTRS{idVendor}=="0511", ATTRS{idProduct}=="0029",DEVPATH=="/devices/pci0000:00/0000:00:08.1/0000:05:00.3/usb1/1-4/1-4.4", MODE="0664", GROUP="video", SYMLINK+="FSUSB2N_2"  

## [mirakurun tuners.ymlの内容例]  
\- name: FSUSB2N-T1  
  types:  
    \- GR  
  command: recfsusb2n --b25 --dev /dev/FSUSB2N_1 \<channel\> - -  
  decoder:  
  isDisabled: false  

\- name: FSUSB2N-T2  
  types:  
    \- GR  
  command: recfsusb2n --b25 --dev /dev/FSUSB2N_2 \<channel\> - -  
  decoder:  
  isDisabled: false  
