TARGET = HelloWorldFilter
TEMPLATE = lib
CONFIG -= qt

CPP_STD = c++17
CONFIG += $$CPP_STD nostrip debug_info static


DESTDIR = $$PWD/_bin

INCLUDEPATH += ../

DEFINES += _WINDOWS=1

win32 {
	install.target = install
	install.commands = cd $$PWD && install.bat
	QMAKE_EXTRA_TARGETS += install
	PRE_TARGETDEPS += install

	RC_FILE = ResourceWin/HelloWorldFilter/HelloWorldFilter.rc
	DEF_FILE = Source/PlugInCommon/Win/TriglavPluginWin.def

	DISTFILES += \
		ResourceWin/HelloWorldFilter/HelloWorldFilter.rc
}

SOURCES += \
	Source/HelloWorldFilter/main.cpp

HEADERS += \

DISTFILES += \
	install.bat

