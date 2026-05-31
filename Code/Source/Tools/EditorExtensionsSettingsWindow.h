#pragma once

#include <QMainWindow>
#include <QMap>

class QLineEdit;
class QSplitter;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace EditorExtensions
{
    /// The Heathen Settings window — a non-dockable QMainWindow modelled after
    /// Unity's Project Settings panel.
    ///
    /// Layout:
    ///   ┌─ Settings ─────────────────────────────────────────────────────────┐
    ///   │ [🔍 search...                                                    ] │
    ///   ├─────────────────────┬──────────────────────────────────────────────┤
    ///   │  Gameplay Tags      │  [page widget for selected item]             │
    ///   │  ▾ Player           │                                              │
    ///   │    Steamworks       │                                              │
    ///   │  Localisation       │                                              │
    ///   │    Lexicon          │                                              │
    ///   └─────────────────────┴──────────────────────────────────────────────┘
    ///
    /// Nav tree is built from ISettingsPage::GetPath() values registered via
    /// ISettingsPageRegistry. Paths use "/" as a separator for nesting.
    ///
    /// Singleton: call ShowOrRaise() to open or bring to front.
    class EditorExtensionsSettingsWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit EditorExtensionsSettingsWindow(QWidget* parent = nullptr);
        ~EditorExtensionsSettingsWindow() override;

        /// Opens the window (creating it if necessary) or brings it to the front.
        static void ShowOrRaise();

        /// Closes the window if it is currently open. Called on editor shutdown.
        static void CloseIfOpen();

    protected:
        void closeEvent(QCloseEvent* event) override;

    private slots:
        void OnNavSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
        void OnSearchTextChanged(const QString& text);

    private:
        void BuildNavAndPages();
        void ApplySearchFilter(const QString& filter);

        /// Ensures the page widget for the given stack index exists (lazy create).
        void EnsurePageCreated(int stackIndex);

        // ── Persistence ───────────────────────────────────────────────────────
        void SaveState();
        void RestoreState();

        QLineEdit*      m_searchField = nullptr;
        QTreeWidget*    m_navTree     = nullptr;
        QSplitter*      m_splitter    = nullptr;
        QStackedWidget* m_pageStack   = nullptr;

        // Maps the full nav path (e.g. "Player/Steamworks") to the page's stack index.
        QMap<QString, int> m_pathToStackIndex;

        // Whether the page widget at each stack index has been created yet.
        QVector<bool> m_pageCreated;

        static EditorExtensionsSettingsWindow* s_instance;
    };

} // namespace EditorExtensions
