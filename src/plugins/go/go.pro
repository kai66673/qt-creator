include(../../qtcreatorplugin.pri)

DEFINES += GOEDITOR_LIBRARY

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
    settings/gogeneralsettings.h \
    settings/gosettings.h \
    settings/gotoolssettingspage.h \
    settings/gotoolssettings.h \
    settings/gotoolssettingspagewidget.h \
    editor/gocompletionassist.h \
    editor/gooutlinewidgetfactory.h \
    tools/control.h \
    tools/diagnosticclient.h \
    tools/gocodemodelmanager.h \
    tools/goeditordocumenthandle.h \
    tools/goeditordocumentparser.h \
    tools/goeditordocumentprocessor.h \
    tools/goeditoroutline.h \
    tools/gooverviewmodel.h \
    tools/gopackage.h \
    tools/gopackagedirindexer.h \
    tools/gopackagetypes.h \
    tools/gosemantichighlighter.h \
    tools/gosource.h \
    tools/literals.h \
    tools/literaltable.h \
    tools/lookupcontext.h \
    tools/memorypool.h \
    tools/methodsscope.h \
    tools/parser.h \
    tools/scanner.h \
    tools/scope.h \
    tools/symbol.h \
    tools/symboltable.h \
    tools/token.h \
    tools/translationunit.h \
    tools/types.h \
    tools/ast.h \
    tools/astfwd.h \
    tools/astvisitor.h \
    editor/gohoverhandler.h \
    tools/gochecksymbols.h \
    tools/scopevisitor.h \
    tools/resolvecontext.h \
    tools/packagetype.h \
    tools/gocompletionassistvisitor.h \
    tools/gofunctionhintassistvisitor.h \
    tools/symbolundercursor.h \
    tools/symbolundercursordescriber.h \
    tools/linkundercursor.h \
    tools/gofindreferences.h \
    tools/findreferences.h \
    tools/goworkingcopy.h \
    tools/resolvedtype.h \
    settings/goconfigurations.h

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
    settings/gogeneralsettings.cpp \
    settings/gosettings.cpp \
    settings/gotoolssettingspage.cpp \
    settings/gotoolssettings.cpp \
    settings/gotoolssettingspagewidget.cpp \
    editor/gohoverhandler.cpp \
    editor/gocompletionassist.cpp \
    editor/gooutlinewidgetfactory.cpp \
    tools/control.cpp \
    tools/diagnosticclient.cpp \
    tools/gocodemodelmanager.cpp \
    tools/goeditordocumenthandle.cpp \
    tools/goeditordocumentparser.cpp \
    tools/goeditordocumentprocessor.cpp \
    tools/goeditoroutline.cpp \
    tools/gooverviewmodel.cpp \
    tools/gopackage.cpp \
    tools/gopackagedirindexer.cpp \
    tools/gopackagetypes.cpp \
    tools/gosemantichighlighter.cpp \
    tools/gosource.cpp \
    tools/literals.cpp \
    tools/memorypool.cpp \
    tools/methodsscope.cpp \
    tools/parser.cpp \
    tools/scanner.cpp \
    tools/scope.cpp \
    tools/symbol.cpp \
    tools/symboltable.cpp \
    tools/token.cpp \
    tools/translationunit.cpp \
    tools/types.cpp \
    tools/ast.cpp \
    tools/astvisitor.cpp \
    tools/gochecksymbols.cpp \
    tools/scopevisitor.cpp \
    tools/resolvecontext.cpp \
    tools/packagetype.cpp \
    tools/gocompletionassistvisitor.cpp \
    tools/gofunctionhintassistvisitor.cpp \
    tools/symbolundercursor.cpp \
    tools/symbolundercursordescriber.cpp \
    tools/linkundercursor.cpp \
    tools/gofindreferences.cpp \
    tools/findreferences.cpp \
    tools/goworkingcopy.cpp \
    tools/resolvedtype.cpp \
    settings/goconfigurations.cpp

FORMS += \
    project/gocompilerbuildstepconfigwidget.ui \
    settings/gogeneralsettingspagewidget.ui \
    settings/gotoolssettingspagewidget.ui
