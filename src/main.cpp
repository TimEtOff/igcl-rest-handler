#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <future>
#include <cstdio>
#include <magic_enum/magic_enum.hpp>
#include <QApplication>
#include <SingleApplication.h>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <memory>
#include <qdialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <string>
#include <thread>
#include "http_server.hpp"

#define APP_VERSION "v0.1.0"

SingleApplication *app;
QIcon *appIcon;
QSystemTrayIcon *trayIcon;
QMenu *menu;
QAction *portAction;
QAction *allowEditAction;
QAction *allowOverclockAction;
QAction *aboutAction;
QAction *closeAction;

std::future<int> httpServerThread;

std::atomic<bool> runServer;
std::atomic<unsigned short> serverPort;
std::atomic<bool> allowEdit;
std::atomic<bool> allowOverclock;
std::shared_ptr<std::basic_ofstream<char, std::char_traits<char>>> appLog;

void attachParentConsole()
{
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        return;
    }

    FILE *dummy = nullptr;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
}

std::string getReadableVersion(uint64_t integer)
{
    ULARGE_INTEGER version;
    version.QuadPart = integer;
    std::string str;
    str += std::to_string(HIWORD(version.HighPart)) + ".";
    str += std::to_string(LOWORD(version.HighPart)) + ".";
    str += std::to_string(HIWORD(version.LowPart)) + ".";
    str += std::to_string(LOWORD(version.LowPart));

    return str;
}

void initLog()
{
    std::string logPath = (QApplication::applicationDirPath() + "/igcl-rest-handler.log").toStdString();

    std::ifstream oldLogR(logPath);
    if (oldLogR) {
        std::ofstream oldLogW(logPath + ".old");

        std::string line;
        while (std::getline(oldLogR, line))
            oldLogW << line << std::endl;

        oldLogW.close();
    }
    oldLogR.close();

    appLog = std::make_shared<std::ofstream>(logPath);
}

// Source - https://stackoverflow.com/a/8196291
// Posted by Beached
// Retrieved 2026-08-05, License - CC BY-SA 3.0
BOOL IsElevated( ) {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess( ),TOKEN_QUERY,&hToken ) ) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if( hToken ) {
        CloseHandle( hToken );
    }
    return fRet;
}

void startServer()
{
    runServer = true;

    httpServerThread = std::async(std::launch::async, http_run);

    portAction->setText(QString::fromStdString("Port: " + std::to_string(static_cast<unsigned short>(serverPort))));
}

void changePort()
{
    //bool ok = false;
    //int nPort = QInputDialog::getInt(nullptr, , , serverPort, 0, , 1, &ok);
    auto *dialog = new QInputDialog(nullptr);
    dialog->setWindowTitle("Change port");
    dialog->setWindowIcon(*appIcon);
    dialog->setLabelText("Change server port to:");
    dialog->setIntMinimum(0);
    dialog->setIntMaximum(65535);
    dialog->setIntValue(static_cast<int>(serverPort));
    dialog->setModal(true);

    if (dialog->exec() == QDialog::Accepted) {
        runServer = false;
        serverPort = dialog->intValue();

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        startServer();
    }
}

void toggledEdit(bool enabled)
{
    allowEdit = enabled;
}

void toggledOverclock(bool enabled)
{
    if (enabled) {
        int validation = QMessageBox::question(nullptr,
            "Overclocking",
            "Overclocking can cause system instability and reduce your device lifespan. Enable it only if you know what you are doing. Do you want to allow it ?");
        if (validation == QMessageBox::Yes)
            allowOverclock = true;
        else
            allowOverclock = false;
    } else
        allowOverclock = false;

    allowOverclockAction->setChecked(allowOverclock);
}

void openGithubPage()
{
    QDesktopServices::openUrl(QUrl("https://github.com/TimEtOff/igcl-rest-handler"));
}

void closeApp()
{
    runServer = false;
    int serverRes = httpServerThread.get();

    info("Closing app...", "app");
    appLog->close();
    app->exit(serverRes);
}

int main(int argc, char *argv[])
{
    attachParentConsole();

    app = new SingleApplication(argc, argv);
    app->setQuitOnLastWindowClosed(false);
    appIcon = new QIcon(":/images/app.ico");

    menu = new QMenu();

    portAction = menu->addAction("&Port: ");
    QObject::connect(portAction, &QAction::triggered, &changePort);

    menu->addSeparator();

    allowEditAction = menu->addAction("Allow &edits");
    allowEditAction->setCheckable(true);
    allowEditAction->setChecked(allowEdit);
    QObject::connect(allowEditAction, &QAction::toggled, &toggledEdit);

    allowOverclockAction = menu->addAction("Allow &overclocking");
    allowOverclockAction->setCheckable(true);
    allowOverclockAction->setChecked(false);
    QObject::connect(allowOverclockAction, &QAction::toggled, &toggledOverclock);

    menu->addSeparator();

    aboutAction = menu->addAction("&About");
    QObject::connect(aboutAction, &QAction::triggered, &openGithubPage);

    closeAction = menu->addAction("&Close");
    QObject::connect(closeAction, &QAction::triggered, &closeApp);

    trayIcon = new QSystemTrayIcon(app);
    trayIcon->setToolTip(QString::fromStdString(std::string("IGCL REST Handler ") + std::string(APP_VERSION)));
    trayIcon->setIcon(*appIcon);
    trayIcon->setContextMenu(menu);

    serverPort = 9738;
    allowEdit = true;

    startServer();

    allowEditAction->setChecked(allowEdit);

    initLog();

    if (IsElevated())
        info("App started as administrator", "app");
    else {
        QMessageBox::warning(nullptr, "Not started as administrator",
            "IGCL REST Handler was not started as administator. This might cause some features to not work correctly.");
        info("App not started as administrator. Some issues might appear", "app");
    }

    trayIcon->show();
    return app->exec();
}
