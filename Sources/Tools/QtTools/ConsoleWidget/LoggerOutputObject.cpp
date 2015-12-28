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


#include "LoggerOutputObject.h"
#include "Debug/DVAssert.h"

class LoggerOutputObject::LoggerOutputContainer : private DAVA::LoggerOutput, public QObject
{
public:
    LoggerOutputContainer(LoggerOutputObject* parent);

private slots:
    void AboutToBeDestroyed();

private:
    ~LoggerOutputContainer() override;
    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;
    LoggerOutputObject* parent = nullptr;
};

LoggerOutputObject::LoggerOutputContainer::LoggerOutputContainer(LoggerOutputObject* parent_)
    : parent(parent_)
{
    DAVA::Logger::AddCustomOutput(this);
    DVASSERT(nullptr != parent);
    connect(parent, &QObject::destroyed, this, &LoggerOutputContainer::AboutToBeDestroyed);
}

void LoggerOutputObject::LoggerOutputContainer::AboutToBeDestroyed()
{
    DAVA::Logger::RemoveCustomOutput(this); //as a static method, must be safe for Logger::Instance() == nullptr
}

LoggerOutputObject::LoggerOutputContainer::~LoggerOutputContainer()
{
    parent->outputContainer = nullptr;
}

void LoggerOutputObject::LoggerOutputContainer::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    parent->OutputReady(ll, text);
}

LoggerOutputObject::LoggerOutputObject(QObject* parent)
    : QObject(parent)
    , outputContainer(new LoggerOutputContainer(this))
{
    qRegisterMetaType<DAVA::Logger::eLogLevel>("DAVA::Logger::eLogLevel");
}