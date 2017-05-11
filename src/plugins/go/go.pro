include(../../qtcreatorplugin.pri)

DEFINES += GOEDITOR_LIBRARY
CONFIG += c++11

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/editor
INCLUDEPATH += $$PWD/project
INCLUDEPATH += $$PWD/settings
INCLUDEPATH += $$PWD/tools

RESOURCES += \
    go.qrc

HEADERS += \
    goplugin.h \
    goiconprovider.h \
    editor/golexer.h \
    editor/gotoken.h \
    goconstants.h \
    editor/goeditorconstants.h \
    editor/gohighlighter.h \
    editor/goindenter.h \
    editor/gocodeformatter.h \
    editor/goautocompleter.h \
    editor/goeditor.h \
    editor/godocument.h \
    project/gobuildconfiguration.h \
    project/gobuildconfigurationfactory.h \
    project/gobuildconfigurationwidget.h \
    project/gocompilerbuildstep.h \
    project/gocompilerbuildstepconfigwidget.h \
    project/gocompilerbuildstepfactory.h \
    project/gocompilercleanstepfactory.h \
    project/gocompilergetstepfactory.h \
    settings/gogeneralsettingspage.h \
    settings/gogeneralsettingspagewidget.h \
    project/golangconstants.h \
    project/golangjsextensions.h \
    project/gooutputparser.h \
    project/gopackageprocessor.h \
    project/goproject.h \
    project/goprojectnode.h \
    project/gorunconfiguration.h \
    project/gorunconfigurationfactory.h \
    project/gorunconfigurationwidget.h \
    project/gotoolchain.h \
    project/gotoolchainconfigwidget.h \
    tools/gosource.h \
    tools/control.h \
    tools/diagnosticclient.h \
    tools/translationunit.h \
    tools/literals.h \
    tools/token.h \
    tools/parser.h \
    tools/memorypool.h \
    tools/ast.h \
    tools/astfwd.h \
    tools/astvisitor.h \
    tools/scanner.h \
    tools/symbol.h \
    tools/symboltable.h \
    tools/scope.h \
    tools/literaltable.h \
    settings/gogeneralsettings.h \
    settings/gosettings.h \
    settings/gotoolssettingspage.h \
    settings/gotoolssettings.h \
    settings/gotoolssettingspagewidget.h \
    tools/gocodemodelmanager.h \
    tools/goeditordocumenthandle.h \
    tools/goeditordocumentprocessor.h \
    tools/goeditordocumentparser.h \
    tools/gooverviewmodel.h \
    tools/goeditoroutline.h \
    tools/gopackage.h \
    tools/gopackageimporter.h \
    tools/gopackagetypes.h \
    tools/gosemantichighlighter.h \
    tools/gosnapshot.h \
    tools/gochecksymbols.h \
    editor/gohoverhandler.h \
    tools/lookupcontext.h \
    tools/methodsscope.h \
    tools/types.h \
    tools/exprtyperesolver.h \
    editor/gocompletionassist.h \
    tools/gocompletionassistvisitor.h \
    tools/gopackagedirindexer.h \
    tools/symbolundercursor.h \
    editor/gooutlinewidgetfactory.h

SOURCES += \
    goplugin.cpp \
    goiconprovider.cpp \
    editor/golexer.cpp \
    editor/gotoken.cpp \
    editor/gohighlighter.cpp \
    editor/goindenter.cpp \
    editor/gocodeformatter.cpp \
    editor/goautocompleter.cpp \
    editor/goeditor.cpp \
    editor/godocument.cpp \
    project/gobuildconfiguration.cpp \
    project/gobuildconfigurationfactory.cpp \
    project/gobuildconfigurationwidget.cpp \
    project/gocompilerbuildstep.cpp \
    project/gocompilerbuildstepconfigwidget.cpp \
    project/gocompilerbuildstepfactory.cpp \
    project/gocompilercleanstepfactory.cpp \
    project/gocompilergetstepfactory.cpp \
    settings/gogeneralsettingspage.cpp \
    settings/gogeneralsettingspagewidget.cpp \
    project/golangjsextensions.cpp \
    project/gooutputparser.cpp \
    project/gopackageprocessor.cpp \
    project/goproject.cpp \
    project/goprojectnode.cpp \
    project/gorunconfiguration.cpp \
    project/gorunconfigurationfactory.cpp \
    project/gorunconfigurationwidget.cpp \
    project/gotoolchain.cpp \
    project/gotoolchainconfigwidget.cpp \
    tools/gosource.cpp \
    tools/control.cpp \
    tools/diagnosticclient.cpp \
    tools/translationunit.cpp \
    tools/literals.cpp \
    tools/token.cpp \
    tools/parser.cpp \
    tools/memorypool.cpp \
    tools/ast.cpp \
    tools/astvisitor.cpp \
    tools/scanner.cpp \
    tools/symbol.cpp \
    tools/symboltable.cpp \
    tools/scope.cpp \
    settings/gogeneralsettings.cpp \
    settings/gosettings.cpp \
    settings/gotoolssettingspage.cpp \
    settings/gotoolssettings.cpp \
    settings/gotoolssettingspagewidget.cpp \
    tools/gocodemodelmanager.cpp \
    tools/goeditordocumenthandle.cpp \
    tools/goeditordocumentprocessor.cpp \
    tools/goeditordocumentparser.cpp \
    tools/gooverviewmodel.cpp \
    tools/goeditoroutline.cpp \
    tools/gopackage.cpp \
    tools/gopackageimporter.cpp \
    tools/gopackagetypes.cpp \
    tools/gosemantichighlighter.cpp \
    tools/gosnapshot.cpp \
    tools/gochecksymbols.cpp \
    editor/gohoverhandler.cpp \
    tools/methodsscope.cpp \
    tools/types.cpp \
    tools/exprtyperesolver.cpp \
    editor/gocompletionassist.cpp \
    tools/gocompletionassistvisitor.cpp \
    tools/gopackagedirindexer.cpp \
    tools/symbolundercursor.cpp \
    editor/gooutlinewidgetfactory.cpp

FORMS += \
    project/gocompilerbuildstepconfigwidget.ui \
    settings/gogeneralsettingspagewidget.ui \
    settings/gotoolssettingspagewidget.ui
