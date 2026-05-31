#pragma once

namespace EditorExtensions
{
    // Module TypeIds
    inline constexpr const char* EditorExtensionsModuleTypeId       = "{8C0F3B52-D694-4E78-C230-B956F4A71839}";
    inline constexpr const char* EditorExtensionsEditorModuleTypeId = EditorExtensionsModuleTypeId; // mutually exclusive

    // Component TypeIds
    inline constexpr const char* EditorExtensionsEditorComponentTypeId = "{5A8F3C72-D1E4-4B96-A037-C52E89D7F614}";

    // Interface TypeIds
    inline constexpr const char* ISettingsPageTypeId         = "{9D1A4C63-E7A5-4F89-D341-CA67E5B8294A}";
    inline constexpr const char* ISettingsPageRegistryTypeId = "{AE2B5D74-F8B6-4A9C-E452-DB78F6C93A5B}";

} // namespace EditorExtensions
