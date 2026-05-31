#pragma once

#include <AzCore/Interface/Interface.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>  // needed for RegisterPage parameter
#include <EditorExtensions/ISettingsPage.h>

namespace EditorExtensions
{
    /// Central registry for Settings window pages.
    ///
    /// Implemented by EditorExtensionsEditorComponent and exposed via AZ::Interface<>.
    /// Gems register pages in their EditorSystemComponent::Activate():
    ///
    ///   if (auto* reg = AZ::Interface<ISettingsPageRegistry>::Get())
    ///       reg->RegisterPage(AZStd::make_unique<LexiconSettingsPage>());
    ///
    /// The Settings window reads GetPages() when it opens to build the nav tree.
    class ISettingsPageRegistry
    {
    public:
        AZ_RTTI(ISettingsPageRegistry, "{AE2B5D74-F8B6-4A9C-E452-DB78F6C93A5B}");

        virtual ~ISettingsPageRegistry() = default;

        /// Register a page. The registry takes ownership of the page object.
        /// Safe to call before the Settings window has been opened.
        virtual void RegisterPage(AZStd::unique_ptr<ISettingsPage> page) = 0;

        /// Returns all registered pages in registration order (non-owning raw pointers).
        virtual const AZStd::vector<ISettingsPage*>& GetPages() const = 0;
    };

} // namespace EditorExtensions
