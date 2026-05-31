#include <EditorExtensions/EditorExtensionsTypeIds.h>
#include "EditorExtensionsEditorComponent.h"

#include <AzCore/Module/Module.h>

namespace EditorExtensions
{
    class EditorExtensionsEditorModule : public AZ::Module
    {
    public:
        AZ_RTTI(EditorExtensionsEditorModule, EditorExtensionsEditorModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(EditorExtensionsEditorModule, AZ::SystemAllocator);

        EditorExtensionsEditorModule()
        {
            m_descriptors.insert(m_descriptors.end(), {
                EditorExtensionsEditorComponent::CreateDescriptor(),
            });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<EditorExtensionsEditorComponent>(),
            };
        }
    };

} // namespace EditorExtensions

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), EditorExtensions::EditorExtensionsEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_EditorExtensions_Editor, EditorExtensions::EditorExtensionsEditorModule)
#endif
