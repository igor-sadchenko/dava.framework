#include "Classes/PropertyPanel/Private/KeyedArchiveEditors.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Deprecated/EditorConfig.h"
#include "Classes/Commands2/KeyedArchiveCommand.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/Widget.h>
#include <TArc/Controls/CommonStrings.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <FileSystem/KeyedArchive.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Functional/Signal.h>
#include <Base/RefPtr.h>

#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QtEvents>

namespace PropertyPanel
{
class AddKeyedArchiveItemWidget : public QWidget
{
public:
    AddKeyedArchiveItemWidget(DAVA::TArc::ContextAccessor* accessor_, DAVA::TArc::UI* ui, const DAVA::TArc::WindowKey& wndKey,
                              DAVA::Vector<DAVA::RefPtr<DAVA::KeyedArchive>>&& archives_, DAVA::int32 lastAddedType)
        : accessor(accessor_)
        , archives(std::move(archives_))
        , type(lastAddedType)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        PrepareData();

        if (type <= VariantType::TYPE_NONE || type >= VariantType::TYPES_COUNT)
        {
            type = VariantType::TYPE_STRING;
        }

        QGridLayout* layout = new QGridLayout(this);
        layout->setMargin(5);
        layout->setSpacing(3);

        layout->addWidget(new QLabel("Key:", this), 0, 0, 1, 1);
        layout->addWidget(new QLabel("Value type:", this), 1, 0, 1, 1);
        layout->addWidget(new QLabel("Preset:", this), 2, 0, 1, 1);

        Reflection r = Reflection::Create(ReflectedObject(this));

        {
            LineEdit::Params params(accessor, ui, wndKey);
            params.fields[LineEdit::Fields::IsEnabled] = "isKeyEnabled";
            params.fields[LineEdit::Fields::Text] = "key";
            LineEdit* keyEdit = new LineEdit(params, accessor, r, this);
            keyEdit->SetObjectName(QString("keyEdit"));
            layout->addWidget(keyEdit->ToWidgetCast(), 0, 1, 1, 2);
            lineEdit = keyEdit->ToWidgetCast();
            setFocusProxy(lineEdit);
        }

        {
            ComboBox::Params params(accessor, ui, wndKey);
            params.fields[ComboBox::Fields::Enumerator] = "types";
            params.fields[ComboBox::Fields::Value] = "type";
            params.fields[ComboBox::Fields::IsReadOnly] = "isTypeDisabled";
            ComboBox* typesCombo = new ComboBox(params, accessor, r, this);
            layout->addWidget(typesCombo->ToWidgetCast(), 1, 1, 1, 2);
        }

        {
            ComboBox::Params params(accessor, ui, wndKey);
            params.fields[ComboBox::Fields::Enumerator] = "presets";
            params.fields[ComboBox::Fields::Value] = "presetIndex";
            ComboBox* typesCombo = new ComboBox(params, accessor, r, this);
            layout->addWidget(typesCombo->ToWidgetCast(), 2, 1, 1, 2);
        }

        QPushButton* button = new QPushButton(QStringLiteral("Add property"), this);
        button->setObjectName("addPropertyButton");
        connections.AddConnection(button, &QPushButton::clicked, MakeFunction(this, &AddKeyedArchiveItemWidget::OnButtonClicked));
        layout->addWidget(button, 3, 1, 1, 2);

        setAttribute(Qt::WA_DeleteOnClose);
        setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
        setWindowOpacity(0.95);
    }

    void Show()
    {
        show();
        lineEdit->setFocus();
    }

    DAVA::Signal<const DAVA::String&, const DAVA::VariantType&> commitAddPropperty;

private:
    void keyPressEvent(QKeyEvent* e)
    {
        if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter))
        {
            switch (e->key())
            {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                OnButtonClicked();
                break;
            case Qt::Key_Escape:
                this->deleteLater();
                break;
            default:
                e->ignore();
                return;
            }
        }
        else
        {
            e->ignore();
        }
    }

    void OnButtonClicked()
    {
        if (key.empty())
        {
            return;
        }

        if (presetIndex != 0)
        {
            ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
            DVASSERT(data);

            const EditorConfig* editorConfig = data->GetEditorConfig();
            const DAVA::VariantType* v = editorConfig->GetPropertyDefaultValue(key);
            commitAddPropperty.Emit(key, *v);
        }
        else
        {
            commitAddPropperty.Emit(key, DAVA::VariantType::FromType(DAVA::VariantType::variantNamesMap[type].variantMeta));
        }

        close();
    }

    void PrepareData()
    {
        ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
        DVASSERT(data);

        const EditorConfig* editorConfig = data->GetEditorConfig();
        const DAVA::Vector<DAVA::String>& presetValues = editorConfig->GetProjectPropertyNames();
        presets.push_back("None");
        presets.insert(presets.end(), presetValues.begin(), presetValues.end());

        for (DAVA::int32 type = DAVA::VariantType::TYPE_NONE + 1; type < DAVA::VariantType::TYPES_COUNT; type++)
        {
            bool ignoreType = false;
            switch (type)
            {
            case DAVA::VariantType::TYPE_BYTE_ARRAY:
            case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
            case DAVA::VariantType::TYPE_MATRIX2:
            case DAVA::VariantType::TYPE_MATRIX3:
            case DAVA::VariantType::TYPE_MATRIX4:
            case DAVA::VariantType::TYPE_INT8:
            case DAVA::VariantType::TYPE_UINT8:
            case DAVA::VariantType::TYPE_INT16:
            case DAVA::VariantType::TYPE_UINT16:
            case DAVA::VariantType::TYPE_INT64:
            case DAVA::VariantType::TYPE_UINT64:
                ignoreType = true;
            default:
                break;
            }

            if (ignoreType == true)
            {
                continue;
            }

            types[type] = DAVA::VariantType::variantNamesMap[type].variantName;
        }
    }

    const DAVA::String& GetKey() const
    {
        return key;
    }

    void SetKey(const DAVA::String& key_)
    {
        key = key_;
    }

    bool IsKeyEnabled() const
    {
        return presetIndex == 0;
    }

    bool IsTypeDisabled() const
    {
        return presetIndex != 0;
    }

    DAVA::int32 GetPreset() const
    {
        return presetIndex;
    }

    void SetPreset(DAVA::int32 preset)
    {
        presetIndex = preset;
        if (presetIndex == 0)
        {
            key.clear();
        }
        else
        {
            SetKey(presets[presetIndex]);

            ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
            DVASSERT(data);

            const EditorConfig* editorConfig = data->GetEditorConfig();
            type = editorConfig->GetPropertyValueType(key);
        }
    }

    DAVA::String key = "";
    DAVA::int32 type = DAVA::VariantType::TYPE_STRING;
    DAVA::int32 presetIndex = 0;

    DAVA::Map<DAVA::int32, DAVA::String> types;
    DAVA::Vector<DAVA::String> presets;

    DAVA::TArc::ContextAccessor* accessor;
    DAVA::Vector<DAVA::RefPtr<DAVA::KeyedArchive>> archives;
    QWidget* lineEdit = nullptr;

    DAVA::TArc::QtConnections connections;

    DAVA_REFLECTION(AddKeyedArchiveItemWidget);
};

DAVA_REFLECTION_IMPL(AddKeyedArchiveItemWidget)
{
    DAVA::ReflectionRegistrator<AddKeyedArchiveItemWidget>::Begin()
    .Field("key", &AddKeyedArchiveItemWidget::GetKey, &AddKeyedArchiveItemWidget::SetKey)
    .Field("isKeyEnabled", &AddKeyedArchiveItemWidget::IsKeyEnabled, nullptr)
    .Field("type", &AddKeyedArchiveItemWidget::type)
    .Field("isTypeDisabled", &AddKeyedArchiveItemWidget::IsTypeDisabled, nullptr)
    .Field("types", &AddKeyedArchiveItemWidget::types)
    .Field("presetIndex", &AddKeyedArchiveItemWidget::GetPreset, &AddKeyedArchiveItemWidget::SetPreset)
    .Field("presets", &AddKeyedArchiveItemWidget::presets)
    .End();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      KeyedArchiveEditor                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

KeyedArchiveEditor::~KeyedArchiveEditor()
{
    if (widget.isNull() == false)
    {
        widget->commitAddPropperty.DisconnectAll();
        widget->deleteLater();
    }
}

DAVA::Any KeyedArchiveEditor::GetMultipleValue() const
{
    return DAVA::Any();
}

bool KeyedArchiveEditor::IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const
{
    return false;
}

DAVA::TArc::ControlProxy* KeyedArchiveEditor::CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    Widget* w = new Widget(parent);
    QHBoxLayout* layout = new QHBoxLayout(w->ToWidgetCast());
    layout->setMargin(0);
    QToolButton* button = new QToolButton();
    button->setIcon(SharedIcon(":/QtIcons/keyplus.png"));
    button->setIconSize(toolButtonIconSize);
    button->setToolTip("Add keyed archive member");
    button->setAutoRaise(false);

    connections.AddConnection(button, &QToolButton::clicked, MakeFunction(this, &KeyedArchiveEditor::OnButtonClicked));
    layout->addWidget(button, 0, Qt::AlignLeft);

    return w;
}

void KeyedArchiveEditor::OnButtonClicked()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    Vector<RefPtr<KeyedArchive>> archives;
    archives.reserve(nodes.size());
    for (const std::shared_ptr<PropertyNode>& node : nodes)
    {
        KeyedArchive* archive = node->field.ref.GetValue().Cast<KeyedArchive*>();
        archives.push_back(RefPtr<KeyedArchive>::ConstructWithRetain(archive));
    }

    if (widget == nullptr)
    {
        widget = new AddKeyedArchiveItemWidget(GetAccessor(), GetUI(), GetWindowKey(), std::move(archives), lastAddedType);
        widget->commitAddPropperty.Connect(this, &KeyedArchiveEditor::AddProperty);
    }

    widget->Show();

    QWidget* thisWidget = editorWidget->ToWidgetCast();
    QPoint topLeft = thisWidget->mapToGlobal(QPoint(0, 0));

    QRect wRect = widget->geometry();
    QPoint wPos = QPoint(topLeft.x() - wRect.width(), topLeft.y());

    widget->move(wPos);
}

void KeyedArchiveEditor::AddProperty(const DAVA::String& key, const DAVA::VariantType& value)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(Format("Add property %s", key.c_str()), static_cast<uint32>(nodes.size()));
    for (const std::shared_ptr<PropertyNode>& node : nodes)
    {
        KeyedArchive* archive = node->field.ref.GetValue().Cast<KeyedArchive*>();
        if (archive->Count(key) == 0)
        {
            cmdInterface.Exec(std::make_unique<KeyedArchiveAddValueCommand>(archive, key, value));
        }
    }
}

int KeyedArchiveEditor::lastAddedType = DAVA::VariantType::TYPE_STRING;

KeyedArchiveComboPresetEditor::KeyedArchiveComboPresetEditor(const DAVA::Vector<DAVA::Any>& values)
    : allowedValues(values)
{
}

DAVA::Any KeyedArchiveComboPresetEditor::GetMultipleValue() const
{
    return DAVA::Any();
}

bool KeyedArchiveComboPresetEditor::IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const
{
    if (newValue.IsEmpty())
    {
        return false;
    }

    if (currentValue.IsEmpty())
    {
        return true;
    }

    int newIntValue = newValue.Cast<DAVA::int32>();
    int currentIntValue = currentValue.Cast<DAVA::int32>();

    return newIntValue != currentIntValue;
}

DAVA::TArc::ControlProxy* KeyedArchiveComboPresetEditor::CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor)
{
    DAVA::TArc::ComboBox::Params params(GetAccessor(), GetUI(), GetWindowKey());
    params.fields[DAVA::TArc::ComboBox::Fields::Value] = "value";
    params.fields[DAVA::TArc::ComboBox::Fields::Enumerator] = "values";
    params.fields[DAVA::TArc::ComboBox::Fields::IsReadOnly] = readOnlyFieldName;

    return new DAVA::TArc::ComboBox(params, wrappersProcessor, model, parent);
}

DAVA::Any KeyedArchiveComboPresetEditor::GetValueAny() const
{
    return GetValue();
}

void KeyedArchiveComboPresetEditor::SetValueAny(const DAVA::Any& newValue)
{
    SetValue(newValue);
}

DAVA_VIRTUAL_REFLECTION_IMPL(KeyedArchiveComboPresetEditor)
{
    DAVA::ReflectionRegistrator<KeyedArchiveComboPresetEditor>::Begin()
    .Field("value", &KeyedArchiveComboPresetEditor::GetValueAny, &KeyedArchiveComboPresetEditor::SetValueAny)
    .Field("values", &KeyedArchiveComboPresetEditor::allowedValues)
    .End();
}

} // namespace PropertyPanel
