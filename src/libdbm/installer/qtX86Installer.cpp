// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "qtX86Installer.h"

#include <XSys>
#include <QDebug>

QtX86Installer::QtX86Installer(QObject *parent) : QtBaseInstaller (parent)
{

}

bool QtX86Installer::installBootload()
{
    qDebug() << "begin install bootloader on" << m_strPartionName;
    m_progressStatus = INSTALLBOOTLOADER;
    XSys::SynExec("partprobe", m_strPartionName);
    //由于前面的命令中会自动挂载系统，而导致如果操作过快会获取挂载点为空，然后在后面再次进行挂载时又挂载失败。因此加一个延时，让系统内核状态同步完成。
    int iTestCount = 3;
    QString installDir;

    do {
        QThread::msleep(2000);
        installDir = XSys::DiskUtil::MountPoint(m_strPartionName);

        if (!installDir.isEmpty()) {
            break;
        }

        iTestCount--;
    } while(iTestCount > 0);

    if (installDir.isEmpty()) {
        XSys::DiskUtil::Mount(m_strPartionName);
        installDir = XSys::DiskUtil::MountPoint(m_strPartionName);
    }

    if (installDir.isEmpty()) {
        qCritical() << "Error::get(Error::USBMountFailed)";
        return false;
    }
    m_strMountName = installDir;

    QString strDisk = XSys::DiskUtil::GetPartitionDisk(m_strPartionName);

    if (strDisk.isEmpty()) {
        return false;
    }

    XSys::Result result = XSys::Syslinux::InstallBootloader(m_strMountName);

    if (!result.isSuccess()) {
        return false;
    }

    result = XSys::Syslinux::InstallMbr(strDisk);

    if (!result.isSuccess()) {
        return false;
    }

    if (!XSys::DiskUtil::SetActivePartion(strDisk, m_strPartionName)) {
        return false;
    }

    XSys::DiskUtil::SetPartionLabel(m_strPartionName, m_strImage);
    return true;
}
