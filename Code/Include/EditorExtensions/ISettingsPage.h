#pragma once

#include <AzCore/std/string/string.h>

class QWidget;

namespace EditorExtensions
{
    /// Interface every settings page must implement.
    ///
    /// Registration:
    ///   Call ISettingsPageRegistry::RegisterPage() from your gem's
    ///   EditorSystemComponent::Activate(), passing a unique_ptr to your page.
    ///
    /// Path syntax:
    ///   "Localisation Lexicon"   -> top-level entry
    ///   "Player/Steamworks"      -> nested under a "Player" group
    ///   "Player/Social/Discord"  -> doubly-nested
    ///
    /// The Settings window builds its left-nav tree from these paths.
    class ISettingsPage
    {
    public:
        virtual ~ISettingsPage() = default;

        /// Nav-tree path for this page, e.g. "Localisation Lexicon" or "Player/Steamworks".
        virtual AZStd::string GetPath() const = 0;

        /// Creates the Qt content widget. Called once when the page is first shown;
        /// the Settings window takes ownership. Parent is the QStackedWidget panel.
        virtual QWidget* CreateWidget(QWidget* parent) = 0;
    };

} // namespace EditorExtensions
