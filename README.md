# Editor Extensions Foundation
![License](https://img.shields.io/badge/License-Apache_2.0-blue?style=flat-square)
![Maintained](https://img.shields.io/badge/Maintained%3F-yes-green?style=flat-square)
![O3DE](https://img.shields.io/badge/O3DE-25.10%20%2B-%2300AEEF?style=flat-square&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCI+PHBhdGggZmlsbD0id2hpdGUiIGQ9Ik0xMiAxTDEgNy40djkuMkwxMiAyM2wxMS02LjRWNy40TDEyIDF6bTkuMSAxNC45TDExLjUgMjEuM2wtOC42LTYuNFY4LjFsOC42LTYuNCA5LjEgNi40djYuOHpNMTEuNSA0LjZMMi45IDkuNnY0LjhsOC42IDUuMSA4LjYtNS4xVjkuNmwtOC42LTUuMHoiLz48L3N2Zz4=)

Foundational editor infrastructure for [Open 3D Engine (O3DE)](https://o3de.org) projects. Provides a unified **Settings window** and a **page-registration API** that lets any Heathen gem contribute its own settings panel — no hard dependencies between gems.

- **License:** Apache 2.0
- **Origin:** Heathen Group
- **Platforms:** Windows, Linux, macOS

> [!TIP]
> **Looking for the easiest way to install?**  
> You can add this gem—along with all of Heathen's free O3DE tools—by using the centralized [O3DE-Gems](https://github.com/heathen-engineering/O3DE-Gems) repository. Step-by-step setup instructions are available directly in its README.

---

## Requirements

- O3DE engine **25.10.2** or compatible

---

## Become a GitHub Sponsor
[![Discord](https://img.shields.io/badge/Discord--1877F2?style=social&logo=discord)](https://discord.gg/6X3xrRc)
[![GitHub followers](https://img.shields.io/github/followers/heathen-engineering?style=social)](https://github.com/heathen-engineering?tab=followers)
Support Heathen by becoming a [GitHub Sponsor](https://github.com/sponsors/heathen-engineering). Sponsorship directly funds the development and maintenance of free tools like this, as well as our game development [Knowledge Base](https://heathen.group/) and community on [Discord](https://discord.gg/6X3xrRc).

Sponsors also get access to our private SourceRepo, which includes developer tools for O3DE, Unreal, Unity, and Godot.
Learn more or explore other ways to support @ [heathen.group/kb](https://heathen.group/kb/do-more/)

---

## What it does

Editor Extensions gives every Heathen gem a shared foundation for editor tooling:

| Component | Purpose |
|-----------|---------|
| **Heathen menu** | Top-level menu entry in the O3DE editor menu bar (between Tools and View) |
| **Settings window** | Non-dockable panel modelled after Unity's Project Settings — nav tree on the left, page content on the right |
| `ISettingsPageRegistry` | `AZ::Interface<>` singleton other gems call to register their settings pages |
| `ISettingsPage` | Interface each settings page must implement to provide its nav-tree path and Qt widget |

The Settings window is opened via **Heathen → Settings** in the menu bar. It supports live search filtering of the nav tree, lazy widget creation, and persists window geometry, splitter position, and last-selected page across editor sessions.

---

## Setup

### 1. Add the gem to your O3DE project

Register the gem with the O3DE Project Manager, or add it directly to your project's `project.json`:

```json
"gem_names": [
    "EditorExtensions"
]
```

Then re-run CMake configuration so the build system picks up the new gem.

---

## Settings Window

The **Settings** panel (under **Heathen** in the O3DE Editor menu bar) provides a single unified location for all Heathen gem configuration:

```
┌─ Settings ───────────────────────────────────────────────┐
│ [🔍 Search settings...                                 ] │
├──────────────────────┬───────────────────────────────────┤
│  Gameplay Tags       │  [page widget for selected item]  │
│  ▾ Player            │                                   │
│    Steamworks        │                                   │
│  Localisation        │                                   │
│    Lexicon           │                                   │
└──────────────────────┴───────────────────────────────────┘
```

- Nav tree is built from each page's `GetPath()` — segments separated by `/` become nested groups
- Search bar filters the nav tree in real time
- Page widgets are created lazily on first selection
- Window state (geometry, splitter, last page) is restored between sessions

---

## Registering a settings page

### 1. Implement `ISettingsPage`

```cpp
#include <EditorExtensions/ISettingsPage.h>

class MySettingsPage : public EditorExtensions::ISettingsPage
{
public:
    // Top-level entry:      "My Feature"
    // Nested under a group: "Player/My Feature"
    AZStd::string GetPath() const override { return "Player/My Feature"; }

    QWidget* CreateWidget(QWidget* parent) override
    {
        // Build and return your Qt widget.
        // The Settings window takes ownership.
        return new MySettingsWidget(parent);
    }
};
```

### 2. Register from your editor system component

```cpp
#include <EditorExtensions/ISettingsPageRegistry.h>

void MyEditorSystemComponent::Activate()
{
    if (auto* reg = AZ::Interface<EditorExtensions::ISettingsPageRegistry>::Get())
        reg->RegisterPage(AZStd::make_unique<MySettingsPage>());
}
```

Register during `Activate()` — pages are read when the Settings window is first opened, so registration is safe at any point before the user opens the window.

### 3. Add actions to the Heathen menu (optional)

Other gems can contribute their own menu actions to `heathen.menu.main` from their own `OnActionRegistrationHook` and `OnMenuBindingHook` implementations on `AzToolsFramework::ActionManagerRegistrationNotificationBus`.

---

## Public API reference

All public headers live under `Code/Include/EditorExtensions/`.

### `ISettingsPage`

| Method | Description |
|--------|-------------|
| `GetPath()` | Nav-tree path for this page, e.g. `"Localisation Lexicon"` or `"Player/Steamworks"`. Use `/` to create nested groups. |
| `CreateWidget(parent)` | Creates the Qt content widget. Called once on first page selection. The Settings window takes ownership. |

### `ISettingsPageRegistry`

Accessed via `AZ::Interface<ISettingsPageRegistry>::Get()`.

| Method | Description |
|--------|-------------|
| `RegisterPage(page)` | Register a page. The registry takes ownership of the page object. Safe to call before the window has opened. |
| `GetPages()` | Returns all registered pages in registration order (non-owning raw pointers). |

### `EditorExtensionsSettingsWindow`

| Method | Description |
|--------|-------------|
| `ShowOrRaise()` *(static)* | Opens the Settings window, or brings it to the front if already open. |
| `CloseIfOpen()` *(static)* | Closes the window if it is open. Called automatically on editor shutdown. |

---

## Public headers

| Header | Contents |
|--------|----------|
| `EditorExtensions/ISettingsPage.h` | `ISettingsPage` interface |
| `EditorExtensions/ISettingsPageRegistry.h` | `ISettingsPageRegistry` interface |
| `EditorExtensions/EditorExtensionsTypeIds.h` | AZ type ID constants |
