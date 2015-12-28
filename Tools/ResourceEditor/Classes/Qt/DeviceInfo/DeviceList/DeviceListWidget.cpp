/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "DeviceListWidget.h"

#include "ui_DeviceListWidget.h"

#include <QDebug>
#include <QCloseEvent> 
#include <QFileDialog>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTabWidget>

#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"

#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/MemProfController.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/MemProfWidget.h"

using namespace DAVA;

DeviceListWidget::DeviceListWidget( QWidget *parent )
    : QWidget( parent, Qt::Window )
    , ui( new Ui::DeviceListWidget() )
{
    ui->setupUi( this );

    connect( ui->connectDevice, &QPushButton::clicked, this, &DeviceListWidget::connectClicked );
    connect( ui->disconnectDevice, &QPushButton::clicked, this, &DeviceListWidget::disconnectClicked );
    connect( ui->showLog, &QPushButton::clicked, this, &DeviceListWidget::showLogClicked );

    connect(ui->viewDump, &QPushButton::clicked, this, &DeviceListWidget::OnViewDump);
    connect(ui->discoverDevice, &QPushButton::clicked, this, &DeviceListWidget::OnDeviceDiscover);
}

DeviceListWidget::~DeviceListWidget() {}

QTreeView* DeviceListWidget::ItemView()
{
    return ui->view;
}

void DeviceListWidget::OnViewDump()
{
    DAVA::FilePath snapshotDir("~doc:/memory-profiling");
    QString filename = QFileDialog::getOpenFileName(this, "Select dump file", snapshotDir.GetAbsolutePathname().c_str(), "Memory logs (*.mlog)");
    if (!filename.isEmpty())
    {
        std::string s = filename.toStdString();
        MemProfController* obj = new MemProfController(FilePath(s), this);
        if (!obj->IsFileLoaded())
        {
            delete obj;
        }
    }
}

void DeviceListWidget::OnDeviceDiscover()
{
    QString s = ui->ipaddr->text().trimmed();
    if (!s.isEmpty())
    {
        emit deviceDiscoverClicked(s);
    }
}
