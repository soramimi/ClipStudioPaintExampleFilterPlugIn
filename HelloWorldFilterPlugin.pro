TEMPLATE = lib
TARGET = HelloWorldFilter


DESTDIR = $$PWD/_bin

INCLUDEPATH += ../

DEFINES += _WINDOWS=1

win32 {
	install.target = .buildfile
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

HEADERS +=

DISTFILES += \
	install.bat

