P7ZIP=/root/source/p7zip_9.20.1
P7CPP=$(P7ZIP)/CPP
P7C=$(P7ZIP)/C

TARGET=lib7z.a
all:$(TARGET)
lib:$(TARGET)

CC=gcc
LOCAL_FLAGS=-DUNICODE -D_UNICODE 
P7INC= -I.. -I$(P7ZIP) -I$(P7CPP) -I$(P7CPP)/include_windows -I$(P7CPP)/myWindows
CXXFLAGS=-g -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DNDEBUG -D_REENTRANT \
	-DENV_UNIX -D_7ZIP_LARGE_PAGES $(LOCAL_FLAGS) $(P7INC)
LDFLAGS=-ldl -lpthread


SRCS=\
$(P7CPP)/myWindows/wine_date_and_time.cpp \
$(P7CPP)/Windows/DLL.cpp \
$(P7CPP)/Windows/FileDir.cpp \
$(P7CPP)/Windows/FileFind.cpp \
$(P7CPP)/Windows/FileIO.cpp \
$(P7CPP)/Windows/FileName.cpp \
$(P7CPP)/Windows/PropVariant.cpp \
$(P7CPP)/Windows/PropVariantConversions.cpp \
$(P7CPP)/Common/IntToString.cpp \
$(P7CPP)/Common/MyWindows.cpp \
$(P7CPP)/Common/MyString.cpp \
$(P7CPP)/Common/StringConvert.cpp \
$(P7CPP)/Common/UTFConvert.cpp \
$(P7CPP)/Common/MyVector.cpp \
$(P7CPP)/Common/Wildcard.cpp \
$(P7CPP)/7zip/Common/FileStreams.cpp \
$(P7C)/Threads.c

OBJS= wine_date_and_time.o \
	MyWindows.o \
	DLL.o \
	FileDir.o \
	FileFind.o \
	FileIO.o \
	FileName.o \
	PropVariant.o \
	PropVariantConversions.o \
	IntToString.o \
	MyString.o \
	StringConvert.o \
	UTFConvert.o \
	MyVector.o \
	Wildcard.o \
	FileStreams.o \
	Threads.o \
	p7libzip.o \
	p7zip_callback.o

$(TARGET):$(OBJS)
	$(AR) rc $@ $^

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

wine_date_and_time.o:$(P7CPP)/myWindows/wine_date_and_time.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
DLL.o:$(P7CPP)/Windows/DLL.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
FileDir.o:$(P7CPP)/Windows/FileDir.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
FileFind.o:$(P7CPP)/Windows/FileFind.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
FileIO.o:$(P7CPP)/Windows/FileIO.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
FileName.o:$(P7CPP)/Windows/FileName.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
PropVariant.o:$(P7CPP)/Windows/PropVariant.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
PropVariantConversions.o:$(P7CPP)/Windows/PropVariantConversions.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
IntToString.o:$(P7CPP)/Common/IntToString.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
MyWindows.o:$(P7CPP)/Common/MyWindows.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
MyString.o:$(P7CPP)/Common/MyString.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
StringConvert.o:$(P7CPP)/Common/StringConvert.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
UTFConvert.o:$(P7CPP)/Common/UTFConvert.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
MyVector.o:$(P7CPP)/Common/MyVector.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
Wildcard.o:$(P7CPP)/Common/Wildcard.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
FileStreams.o:$(P7CPP)/7zip/Common/FileStreams.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
Threads.o:$(P7C)/Threads.c
	$(CC) $(CXXFLAGS) -c $^ -o $@

clean:
	$(RM) *.o $(TARGET)
