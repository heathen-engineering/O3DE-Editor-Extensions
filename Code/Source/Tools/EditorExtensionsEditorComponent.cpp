#include "EditorExtensionsEditorComponent.h"
#include "EditorExtensionsSettingsWindow.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzToolsFramework/ActionManager/Action/ActionManagerInterface.h>
#include <AzToolsFramework/ActionManager/Menu/MenuManagerInterface.h>
#include <AzToolsFramework/Editor/ActionManagerIdentifiers/EditorContextIdentifiers.h>
#include <AzToolsFramework/Editor/ActionManagerIdentifiers/EditorMenuIdentifiers.h>

#include <QDesktopServices>
#include <QUrl>

namespace EditorExtensions
{
    AZ_COMPONENT_IMPL(EditorExtensionsEditorComponent, "EditorExtensionsEditorComponent",
        EditorExtensionsEditorComponentTypeId);

    void EditorExtensionsEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<EditorExtensionsEditorComponent, AZ::Component>()
                ->Version(0);
        }
    }

    void EditorExtensionsEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("EditorExtensionsService"));
    }

    void EditorExtensionsEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("EditorExtensionsService"));
    }

    void EditorExtensionsEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void EditorExtensionsEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void EditorExtensionsEditorComponent::Activate()
    {
        AZ::Interface<ISettingsPageRegistry>::Register(this);
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusConnect();
    }

    void EditorExtensionsEditorComponent::Deactivate()
    {
        EditorExtensionsSettingsWindow::CloseIfOpen();
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusDisconnect();
        AZ::Interface<ISettingsPageRegistry>::Unregister(this);
        for (ISettingsPage* page : m_pages)
            delete page;
        m_pages.clear();
    }

    // ── ISettingsPageRegistry ─────────────────────────────────────────────────

    void EditorExtensionsEditorComponent::RegisterPage(AZStd::unique_ptr<ISettingsPage> page)
    {
        if (page)
            m_pages.push_back(page.release());
    }

    const AZStd::vector<ISettingsPage*>& EditorExtensionsEditorComponent::GetPages() const
    {
        return m_pages;
    }

    // ── ActionManagerRegistrationNotificationBus ──────────────────────────────

    void EditorExtensionsEditorComponent::OnMenuRegistrationHook()
    {
        auto* menuManager = AZ::Interface<AzToolsFramework::MenuManagerInterface>::Get();
        if (!menuManager) return;

        // Register the top-level "Heathen" menu once.
        // Guard against double-registration if multiple Heathen gems arrive here first.
        if (!menuManager->IsMenuRegistered("heathen.menu.main"))
        {
            AzToolsFramework::MenuProperties props;
            props.m_name = "Heathen";
            menuManager->RegisterMenu("heathen.menu.main", props);
        }
    }

    void EditorExtensionsEditorComponent::OnMenuBindingHook()
    {
        auto* menuManager = AZ::Interface<AzToolsFramework::MenuManagerInterface>::Get();
        if (!menuManager) return;

        // Bind the Heathen menu to the menu bar between Tools (~400) and View (~500).
        // Guard so only the first component that arrives does the binding.
        static bool s_bound = false;
        if (!s_bound)
        {
            menuManager->AddMenuToMenuBar(
                AZStd::string(EditorIdentifiers::EditorMainWindowMenuBarIdentifier),
                "heathen.menu.main",
                450);
            s_bound = true;
        }

        // Bind all Heathen actions into the menu
        menuManager->AddActionToMenu("heathen.menu.main", "heathen.action.settings",  100);
        menuManager->AddSeparatorToMenu("heathen.menu.main", 200);
        menuManager->AddActionToMenu("heathen.menu.main", "heathen.action.discord",   300);
        menuManager->AddActionToMenu("heathen.menu.main", "heathen.action.knowledge", 400);
    }

    void EditorExtensionsEditorComponent::OnActionRegistrationHook()
    {
        auto* actionManager = AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
        if (!actionManager) return;

        const AZStd::string contextId(EditorIdentifiers::MainWindowActionContextIdentifier);

        // ── Settings ──────────────────────────────────────────────────────────
        {
            AzToolsFramework::ActionProperties props;
            props.m_name        = "Settings";
            props.m_description = "Open the Heathen Settings window";
            props.m_category    = "Heathen";
            actionManager->RegisterAction(contextId, "heathen.action.settings", props,
                []() { EditorExtensionsSettingsWindow::ShowOrRaise(); });
        }

        // ── Discord Support ───────────────────────────────────────────────────
        {
            AzToolsFramework::ActionProperties props;
            props.m_name        = "Discord Support";
            props.m_description = "Open the Heathen Engineering Discord server";
            props.m_category    = "Heathen";
            actionManager->RegisterAction(contextId, "heathen.action.discord", props,
                []() { QDesktopServices::openUrl(QUrl("https://discord.gg/RMGRxJeba6")); });
        }

        // ── Knowledge Base ────────────────────────────────────────────────────
        {
            AzToolsFramework::ActionProperties props;
            props.m_name        = "Knowledge Base";
            props.m_description = "Open the Heathen Engineering knowledge base";
            props.m_category    = "Heathen";
            actionManager->RegisterAction(contextId, "heathen.action.knowledge", props,
                []() { QDesktopServices::openUrl(QUrl("https://heathen.group/knowledge-base")); });
        }
    }

} // namespace EditorExtensions
