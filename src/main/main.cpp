/*
 * $Revision: 224 $ $Date: 2010-03-30 05:50:58 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License.
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>

#ifdef Q_WS_MAC
#include <CoreFoundation/CFBundle.h>
#endif

#include "Cosmographia.h"

#define MAS_DEPLOY 1


int main(int argc, char *argv[])
{
    extern void qt_force_trolltech_settings_to_app_area(bool bVal);
    qt_force_trolltech_settings_to_app_area(true);

    QApplication app(argc, argv);

#if MAS_DEPLOY
#else
    QCoreApplication::setOrganizationName("Periapsis Visual Software");
    QCoreApplication::setOrganizationDomain("periapsisvisual.com");
    QCoreApplication::setApplicationName("Cosmographia");
#endif

    // Useful when we need to know where the data files are:
    //   qDebug() << QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    //   qDebug() << QDesktopServices::storageLocation(QDesktopServices::DataLocation);

    // Set current directory so that we find the needed data files. On the Mac, we
    // just look in the app bundle. On other platforms we make some guesses, since we
    // don't know exactly where the executable will be run from.
    QString dataPath;
    bool foundData = true;
#ifdef Q_WS_MAC
    // On the Mac, load resources from the app bundle. We first check for a directory
    // in a location relative to the executable. If that fails, we'll use the Core
    // Foundation bundle functions as recommended in the Qt docs. The first technique
    // *should* always work.
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    if (QDir("../Resources/data").exists())
    {
        dataPath = "../Resources/data";
    }
    else
    {
        // Note that there is a bug in one of the bundle functions that prevents this from
        // working properly on a Japanese Mac OS X install.
        CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
        const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
        dataPath = QString(pathPtr) + "/Contents/Resources/data";
        CFRelease(appUrlRef);
        CFRelease(macPath);
    }
#else
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    if (QDir("../data").exists())
    {
        dataPath = "../data";
    }
    else if (QDir("../../data").exists())
    {
        // QtCreator builds in the debug/ or release/ directory
        dataPath = "../../data";
    }
    else if (QDir("../../cosmographia/data").exists())
    {
        dataPath = "../../cosmographia/data";
    }
    else if (QDir("../../trunk/data").exists())
    {
        dataPath = "../../trunk/data";
    }
    else if (QDir("./data").exists())
    {
        dataPath = "./data";
    }
    else
    {
        foundData = false;
    }
#endif
    if (!foundData || !QDir::setCurrent(dataPath))
    {
        QMessageBox::warning(NULL, "Missing data", "Data files not found!");
        exit(0);
    }

    Cosmographia mainWindow;
    mainWindow.initialize();
    mainWindow.show();

    return app.exec();
}
