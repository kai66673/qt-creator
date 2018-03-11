import qbs 1.0

QtcPlugin {
    name: "Go"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }

    cpp.includePaths: base.concat([".", "editor", "project", "settings", "tools"])

    Group {
        name: "General"
        files: [
            "goplugin.cpp", "goplugin.h",
            "goiconprovider.cpp", "goiconprovider.h",
            "goconstants.h",
            "go.qrc"
        ]
    }

    Group {
        name: "Editor"
        prefix: "editor/"
        files: [
            "goautocompleter.cpp",
            "goautocompleter.h",
            "gocodeformatter.cpp",
            "gocodeformatter.h",
            "gocompletionassist.cpp",
            "gocompletionassist.h",
            "godocument.cpp",
            "godocument.h",
            "goeditor.cpp",
            "goeditor.h",
            "goeditorconstants.h",
            "gohighlighter.cpp",
            "gohighlighter.h",
            "gohoverhandler.cpp",
            "gohoverhandler.h",
            "goindenter.cpp",
            "goindenter.h",
            "golexer.cpp",
            "golexer.h",
            "gooutlinewidgetfactory.cpp",
            "gooutlinewidgetfactory.h",
            "gotoken.cpp",
            "gotoken.h",
        ]
    }

    Group {
        name: "Project"
        prefix: "project/"
        files: [
            "gobuildconfiguration.cpp", "gobuildconfiguration.h",
            "gobuildconfigurationfactory.cpp", "gobuildconfigurationfactory.h",
            "gobuildconfigurationwidget.cpp", "gobuildconfigurationwidget.h",
            "gocompilerbuildstep.cpp", "gocompilerbuildstep.h",
            "gocompilerbuildstepconfigwidget.cpp", "gocompilerbuildstepconfigwidget.h", "gocompilerbuildstepconfigwidget.ui",
            "gocompilerbuildstepfactory.cpp", "gocompilerbuildstepfactory.h",
            "gocompilercleanstepfactory.cpp", "gocompilercleanstepfactory.h",
            "gocompilergetstepfactory.cpp", "gocompilergetstepfactory.h",
            "golangconstants.h",
            "golangjsextensions.cpp", "golangjsextensions.h",
            "gooutputparser.cpp", "gooutputparser.h",
            "gopackageprocessor.cpp", "gopackageprocessor.h",
            "goproject.cpp", "goproject.h",
            "goprojectnode.cpp", "goprojectnode.h",
            "gorunconfiguration.cpp", "gorunconfiguration.h",
            "gorunconfigurationfactory.cpp", "gorunconfigurationfactory.h",
            "gorunconfigurationwidget.cpp", "gorunconfigurationwidget.h",
            "gotoolchain.cpp", "gotoolchain.h",
            "gotoolchainconfigwidget.cpp", "gotoolchainconfigwidget.h"
        ]
    }

    Group {
        name: "Settings"
        prefix: "settings/"
        files: [
            "gogeneralsettings.cpp", "gogeneralsettings.h",
            "gogeneralsettingspage.cpp", "gogeneralsettingspage.h",
            "gogeneralsettingspagewidget.cpp", "gogeneralsettingspagewidget.h", "gogeneralsettingspagewidget.ui",
            "gosettings.cpp", "gosettings.h",
            "goconfigurations.cpp", "goconfigurations.h", 
            "gotoolssettings.cpp", "gotoolssettings.h",
            "gotoolssettingspage.cpp", "gotoolssettingspage.h",
            "gotoolssettingspagewidget.cpp", "gotoolssettingspagewidget.h", "gotoolssettingspagewidget.ui"
        ]
    }

    Group {
        name: "Tools"
        prefix: "tools/"
        files: [
            "ast.cpp",
            "ast.h",
            "astfwd.h",
            "astvisitor.cpp",
            "astvisitor.h",
            "control.cpp",
            "control.h",
            "diagnosticclient.cpp",
            "diagnosticclient.h",
            "exprtyperesolver.cpp",
            "exprtyperesolver.h",
            "gochecksymbols.cpp",
            "gochecksymbols.h",
            "gocodemodelmanager.cpp",
            "gocodemodelmanager.h",
            "gocompletionassistvisitor.cpp",
            "gocompletionassistvisitor.h",
            "goeditordocumenthandle.cpp",
            "goeditordocumenthandle.h",
            "goeditordocumentparser.cpp",
            "goeditordocumentparser.h",
            "goeditordocumentprocessor.cpp",
            "goeditordocumentprocessor.h",
            "goeditoroutline.cpp",
            "goeditoroutline.h",
            "gofunctionhintassistvisitor.cpp",
            "gofunctionhintassistvisitor.h",
            "gooverviewmodel.cpp",
            "gooverviewmodel.h",
            "gopackage.cpp",
            "gopackage.h",
            "gopackagedirindexer.cpp",
            "gopackagedirindexer.h",
            "gopackageimporter.cpp",
            "gopackageimporter.h",
            "gopackagetypes.cpp",
            "gopackagetypes.h",
            "gosemantichighlighter.cpp",
            "gosemantichighlighter.h",
            "gosnapshot.cpp",
            "gosnapshot.h",
            "gosource.cpp",
            "gosource.h",
            "literals.cpp",
            "literals.h",
            "literaltable.h",
            "lookupcontext.h",
            "memorypool.cpp",
            "memorypool.h",
            "methodsscope.cpp",
            "methodsscope.h",
            "parser.cpp",
            "parser.h",
            "scanner.cpp",
            "scanner.h",
            "scope.cpp",
            "scope.h",
            "scopevisitor.cpp",
            "scopevisitor.h",
            "symbol.cpp",
            "symbol.h",
            "symboltable.cpp",
            "symboltable.h",
            "symbolundercursor.cpp",
            "symbolundercursor.h",
            "token.cpp",
            "token.h",
            "translationunit.cpp",
            "translationunit.h",
            "types.cpp",
            "types.h",
        ]
    }
}
