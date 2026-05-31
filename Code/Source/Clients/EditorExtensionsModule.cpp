/*
 * Copyright (c) 2026 Heathen Engineering Limited
 * Licensed under the Apache License, Version 2.0
 */

#include <AzCore/Module/Module.h>
#include <EditorExtensions/EditorExtensionsTypeIds.h>

namespace EditorExtensions
{
    // Minimal runtime stub — EditorExtensions has no runtime functionality.
    // All content is in the Editor target.
    class EditorExtensionsModule : public AZ::Module
    {
    public:
        AZ_RTTI(EditorExtensionsModule, EditorExtensionsModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(EditorExtensionsModule, AZ::SystemAllocator);

        EditorExtensionsModule() = default;

        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return {};
        }
    };

} // namespace EditorExtensions

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), EditorExtensions::EditorExtensionsModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_EditorExtensions, EditorExtensions::EditorExtensionsModule)
#endif
