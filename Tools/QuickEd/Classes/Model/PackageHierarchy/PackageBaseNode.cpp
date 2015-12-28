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


#include "PackageBaseNode.h"

using namespace DAVA;

PackageBaseNode::PackageBaseNode(PackageBaseNode* parent)
    : parent(parent)
{
}

PackageBaseNode::~PackageBaseNode()
{
    parent = nullptr;
}

int PackageBaseNode::GetIndex(const PackageBaseNode *node) const
{
    for (int i = 0; i < GetCount(); i++)
    {
        if (Get(i) == node)
            return i;
    }
    return -1;
}

PackageBaseNode *PackageBaseNode::GetParent() const
{
    return parent;
}

void PackageBaseNode::SetParent(PackageBaseNode *parent)
{
    this->parent = parent;
}

String PackageBaseNode::GetName() const
{
    return "Unknown";
}

PackageNode *PackageBaseNode::GetPackage()
{
    return parent ? parent->GetPackage() : nullptr;
}

const PackageNode *PackageBaseNode::GetPackage() const
{
    return parent ? parent->GetPackage() : nullptr;
}

UIControl *PackageBaseNode::GetControl() const
{
    return NULL;
}

void PackageBaseNode::debugDump(int depth)
{
    String str;
    for (int i = 0; i < depth; i++)
        str += ' ';
    Logger::Debug("%sNode %s (%s), %d", str.c_str(), GetName().c_str(), typeid(this).name(), this->GetRetainCount());
    for (int i = 0; i < GetCount(); i++)
        Get(i)->debugDump(depth + 2);
}

bool PackageBaseNode::IsEditingSupported() const
{
    return false;
}

bool PackageBaseNode::IsInsertingControlsSupported() const
{
    return false;
}

bool PackageBaseNode::IsInsertingPackagesSupported() const
{
    return false;
}

bool PackageBaseNode::IsInsertingStylesSupported() const
{
    return false;
}

bool PackageBaseNode::CanInsertControl(ControlNode *node, DAVA::int32 pos) const
{
    return false;
}

bool PackageBaseNode::CanInsertStyle(StyleSheetNode *node, DAVA::int32 pos) const
{
    return false;
}

bool PackageBaseNode::CanInsertImportedPackage(PackageNode *package) const
{
    return false;
}

bool PackageBaseNode::CanRemove() const
{
    return false;
}

bool PackageBaseNode::CanCopy() const
{
    return false;
}

bool PackageBaseNode::IsReadOnly() const
{
    return parent ? parent->IsReadOnly() : true;
}

namespace
{
uint32 CalculateDepth(PackageBaseNode* node)
{
    DVASSERT(nullptr != node);
    uint32 depth = 0;
    PackageBaseNode* parent = node->GetParent();
    while (nullptr != parent)
    {
        ++depth;
        parent = parent->GetParent();
    }
    return depth;
}

PackageBaseNode* ReduceDepth(PackageBaseNode* node, uint32 reduceValue)
{
    for (uint32 i = 0; i < reduceValue; ++i)
    {
        DVASSERT(node != nullptr);
        node = node->GetParent();
    }
    return node;
}
} //unnamed namepace

bool CompareByLCA(PackageBaseNode* left, PackageBaseNode* right)
{
    DVASSERT(nullptr != left && nullptr != right);
    if (left == right)
    {
        return false;
    }
    int depthLeft = CalculateDepth(left);
    int depthRight = CalculateDepth(right);

    PackageBaseNode* leftParent = left;
    PackageBaseNode* rightParent = right;

    if (depthLeft > depthRight)
    {
        leftParent = ReduceDepth(leftParent, depthLeft - depthRight);
        if (leftParent == right) // if left is child of right
        {
            return false;
        }
        left = leftParent;
    }
    else
    {
        rightParent = ReduceDepth(rightParent, depthRight - depthLeft);
        if (rightParent == left) //if right is child of left
        {
            return true;
        }
        right = rightParent;
    }

    while (true)
    {
        leftParent = left->GetParent();
        rightParent = right->GetParent();
        if (nullptr == leftParent) //parent can be nullptr if we remove package and than remove it imported package
        {
            return false;
        }
        else if (nullptr == rightParent)
        {
            return true;
        }

        if (leftParent == rightParent)
        {
            return leftParent->GetIndex(left) < leftParent->GetIndex(right);
        }
        left = leftParent;
        right = rightParent;
    }
}
