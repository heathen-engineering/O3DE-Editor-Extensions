#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzToolsFramework/ActionManager/ActionManagerRegistrationNotificationBus.h>

#include <EditorExtensions/EditorExtensionsTypeIds.h>
#include <EditorExtensions/ISettingsPage.h>
#include <EditorExtensions/ISettingsPageRegistry.h>

namespace EditorExtensions
{
    /// Editor lifecycle component for EditorExtensions.
    ///
    /// Responsibilities:
    ///   - Implements ISettingsPageRegistry and exposes it via AZ::Interface<>
    ///     so that other gems can register settings pages from their Activate().
    ///   - Creates the "Heathen" top-level menu in the O3DE menu bar
    ///     (File | Edit | Game | Tools | Heathen | View | Help).
    ///   - Registers "Settings" action (opens EditorExtensionsSettingsWindow)
    ///     and support links (Discord, Knowledge Base) inside the Heathen menu.
    ///
    /// Other gems add their own actions to "heathen.menu.main" from their own
    /// OnActionRegistrationHook / OnMenuBindingHook implementations.
    class EditorExtensionsEditorComponent
        : public AZ::Component
        , public ISettingsPageRegistry
        , public AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(EditorExtensionsEditorComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        // ISettingsPageRegistry
        void RegisterPage(AZStd::unique_ptr<ISettingsPage> page) override;
        const AZStd::vector<ISettingsPage*>& GetPages() const override;

        // ActionManagerRegistrationNotificationBus
        void OnMenuRegistrationHook()   override;
        void OnMenuBindingHook()        override;
        void OnActionRegistrationHook() override;

    private:
        // Owned raw pointers — unique_ptr members break O3DE SerializeContext copy-construction.
        AZStd::vector<ISettingsPage*> m_pages;
    };

} // namespace EditorExtensions
