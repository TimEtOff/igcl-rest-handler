#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <future>
#include <cstdio>
#include <magic_enum/magic_enum.hpp>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QInputDialog>
#include <memory>
#include <qapplication.h>
#include <qcoreapplication.h>
#include <qdialog.h>
#include <qicon.h>
#include <qinputdialog.h>
#include <qsystemtrayicon.h>
#include <qurl.h>
#include <qwidget.h>
#include <string>
#include <thread>
#include "http_server.hpp"

#define APP_VERSION "v0.1.0"


QApplication *app;
QIcon *appIcon;
QSystemTrayIcon *trayIcon;
QMenu *menu;
QAction *portAction;
QAction *allowEditAction;
QAction *closeAction;

std::future<int> httpServerThread;

std::atomic<bool> runServer;
std::atomic<unsigned short> serverPort;
std::atomic<bool> allowEdit;
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

    app = new QApplication(argc, argv);
    app->setQuitOnLastWindowClosed(false);
    appIcon = new QIcon(":/images/app.ico");

    menu = new QMenu();

    portAction = menu->addAction("&Port: ");
    QObject::connect(portAction, &QAction::triggered, &changePort);

    allowEditAction = menu->addAction("Allow &edits");
    allowEditAction->setCheckable(true);
    QObject::connect(allowEditAction, &QAction::toggled, &toggledEdit);

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

    trayIcon->show();
    return app->exec();
}
