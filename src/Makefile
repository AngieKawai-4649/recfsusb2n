
#【ビルド方法】
#1.SOFT-CASを使用
#    1.1 libarib25をセットアップする
#    1.2 softcasをセットアップする
#    1.3 recfsusb2n/srcディレクトリで
#        make SOFTCAS=1
#        sudo make install (recfsusb2nを/usr/local/binにコピー)
#
#2.内蔵カードリーダーを使用
#	2.1 ktv_cardreader libktvcardreader.so をbuild,installする
#    2.2 recfsusb2n/srcディレクトリで
#        make BUILT-IN=1
#        sudo make install
#	2.3 recfsusb2n を実行する前にktv_cardreader を起動しておく
#
#3.外付けカードリーダーを使用
#    3.1 libarib25をセットアップする
#    3.2 recfsusb2n/srcディレクトリで
#        make EXTERNAL=1
#        sudo make install
#
#4.B25解除を行わない場合
#    B25解除を当プログラムで行わず、mirakurun arib-b25-stream-test等で行う場合
#    recfsusb2n/srcに移動し
#    make
#    sudo make install
#    注 B25解除を当プログラムで行わない場合、tssplitter_liteは使用できません
#
#B25処理を変更する為にビルドし直す場合はフルコンパイルする
#例：内蔵カードリーダーからSOFT-CASに変更する場合
#make BUILT-IN=1 clean 
#make SOFTCAS=1
#
#それぞれのbuildによりリンクするカードリーダーライブラリが異なるのでシンボルテーブルを確認する
#ldd recfsusb2n
#
#SOFT-CAS : libarib25.so.0直下にlibsoftcas.so
#BUILT-IN : libarib25.so.0直下にlibktvcardreader.so
#EXTERNAL : libarib25.so.0直下にlibpcsclite.so.1
#

ifdef SOFTCAS
STD_B25 = -DSTD_B25
LIBS    = -larib25 -lsoftcas
else
ifdef BUILT-IN
STD_B25 = -DSTD_B25
DB_CARDREADER = -DDB_CARDREADER
PCSC = `pkg-config libpcsclite --cflags`
LIBS    = -larib25 -lktvcardreader
else
ifdef EXTERNAL 
STD_B25 = -DSTD_B25
LIBS    = -larib25 `pkg-config libpcsclite --libs`
endif
endif
endif

OBJSP	= fsusb2n.o usbops.o em2874-core.o ktv.o IoThread.o
OBJSC	= message.o tssplitter_lite.o utils.o

CXX	= g++
CC	= gcc
#CFLAGS   = -O2 -Wall -Wno-format-truncation -Wno-format-overflow -pthread -D_LARGEFILE_SOURCE
#CFLAGS   = -O2 -Wall -Wno-format-overflow -pthread -D_LARGEFILE_SOURCE
CXXFLAGS = -O2 -Wall -pthread -D_LARGEFILE_SOURCE $(DB_CARDREADER) $(PCSC)
CFLAGS   = -O2 -Wall -pthread -D_LARGEFILE_SOURCE $(DB_CARDREADER) $(PCSC)
ifeq ($(shell getconf LONG_BIT),32)
CFLAGS  += -D_FILE_OFFSET_BITS=64
endif

LIBS	+= -lpthread
#LIBS	+= -lpthread -lboost_system -lboost_thread -lboost_filesystem
#LIBS	+= -lpthread -lboost_thread-mt -lboost_filesystem

PREFIX = /usr/local
TARGET	= recfsusb2n

all: $(TARGET)

clean:
	rm -f $(OBJSP) $(OBJSC) $(TARGET)

uninstall:
	rm -vf $(PREFIX)/bin/$(TARGET)

install: uninstall
	install -m 755 $(TARGET) $(PREFIX)/bin

$(TARGET): $(OBJSP) $(OBJSC)
	$(CXX) -o $(TARGET) $(OBJSP) $(OBJSC) $(LIBS)
ifdef SOFTCAS
	@echo "\n"B25 DECODE USING SOFTCAS"\n"
else
ifdef BUILT-IN
	@echo "\n"B25 DECODE USING BUILT-IN CARDREADER"\n"
else
ifdef EXTERNAL
	@echo "\n"B25 DECODE USING EXTERNAL CARDREADER"\n"
endif
endif
endif
ifndef STD_B25
	@echo "\n"B25 DECODE OFF
	@echo "SOFTCAS             -> make SOFTCAS=1"
	@echo "BUILT-IN CARDREADER -> make BUILT-IN=1"
	@echo "EXTERNAL CARDREADER -> make EXTERNAL=1""\n"
endif

depend:
	$(CXX) -MM $(OBJSP:.o=.cpp) > Makefile.dep
	$(CC) -MM $(OBJSC:.o=.c) >> Makefile.dep

-include Makefile.dep

utils.o :
	$(CC) $(CFLAGS) $(STD_B25) -c -o $@ $(@:.o=.c)

