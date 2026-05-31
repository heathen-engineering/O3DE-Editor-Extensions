#include "EditorExtensionsSettingsWindow.h"

#include <AzCore/Interface/Interface.h>
#include <EditorExtensions/ISettingsPageRegistry.h>
#include <EditorExtensions/ISettingsPage.h>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QScreen>
#include <QSettings>
#include <QSizeGrip>
#include <QSplitter>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace EditorExtensions
{
    EditorExtensionsSettingsWindow* EditorExtensionsSettingsWindow::s_instance = nullptr;

    // ── Construction ──────────────────────────────────────────────────────────

    EditorExtensionsSettingsWindow::EditorExtensionsSettingsWindow(QWidget* parent)
        : QMainWindow(parent)
    {
        // Ensure the window manager gives a standard frame with resize borders.
        // O3DE's editor stylesheet can suppress decorations — reset to plain Qt::Window.
        setWindowFlags(Qt::Window);
        setWindowTitle("Settings");
        setMinimumSize(900, 600);
        resize(1100, 700);

        // Centre on screen
        if (const QScreen* screen = QApplication::primaryScreen())
        {
            const QRect sg = screen->availableGeometry();
            move(sg.center() - rect().center());
        }

        // ── Central widget ────────────────────────────────────────────────────
        auto* central = new QWidget(this);
        auto* rootLayout = new QVBoxLayout(central);
        // Small inset margin so the window border is visible and content
        // doesn't paint right to the edge of the frame.
        rootLayout->setContentsMargins(4, 4, 4, 4);
        rootLayout->setSpacing(4);
        setCentralWidget(central);

        // ── Search bar ────────────────────────────────────────────────────────
        m_searchField = new QLineEdit(central);
        m_searchField->setPlaceholderText("Search settings...");
        m_searchField->setClearButtonEnabled(true);
        m_searchField->setMinimumHeight(28);
        m_searchField->setContentsMargins(6, 0, 6, 0);
        connect(m_searchField, &QLineEdit::textChanged,
                this, &EditorExtensionsSettingsWindow::OnSearchTextChanged);
        rootLayout->addWidget(m_searchField);

        // ── Splitter: nav (left) | page stack (right) ─────────────────────────
        m_splitter = new QSplitter(Qt::Horizontal, central);
        m_splitter->setChildrenCollapsible(false);

        // Left — nav tree
        m_navTree = new QTreeWidget(m_splitter);
        m_navTree->setHeaderHidden(true);
        m_navTree->setRootIsDecorated(true);
        m_navTree->setAnimated(true);
        m_navTree->setUniformRowHeights(true);
        m_navTree->setSelectionMode(QAbstractItemView::SingleSelection);
        m_navTree->setMinimumWidth(180);
        m_navTree->setMaximumWidth(280);
        connect(m_navTree, &QTreeWidget::currentItemChanged,
                this, &EditorExtensionsSettingsWindow::OnNavSelectionChanged);

        // Right — page stack
        m_pageStack = new QStackedWidget(m_splitter);

        m_splitter->addWidget(m_navTree);
        m_splitter->addWidget(m_pageStack);
        m_splitter->setStretchFactor(0, 0);
        m_splitter->setStretchFactor(1, 1);
        m_splitter->setSizes({ 220, 680 });

        rootLayout->addWidget(m_splitter, 1);

        // ── Separator + size grip ─────────────────────────────────────────────
        auto* footerSep = new QFrame(central);
        footerSep->setFrameShape(QFrame::HLine);
        footerSep->setFrameShadow(QFrame::Sunken);
        rootLayout->addWidget(footerSep);

        auto* gripRow = new QHBoxLayout;
        gripRow->setContentsMargins(0, 0, 0, 0);
        gripRow->addStretch();
        gripRow->addWidget(new QSizeGrip(central));
        rootLayout->addLayout(gripRow);

        // ── Populate nav + page slots from registered pages ───────────────────
        BuildNavAndPages();

        // Auto-close when the editor application quits
        connect(qApp, &QCoreApplication::aboutToQuit,
                this, &EditorExtensionsSettingsWindow::close);

        // Restore geometry + last selected page from previous session
        RestoreState();
    }

    EditorExtensionsSettingsWindow::~EditorExtensionsSettingsWindow()
    {
        s_instance = nullptr;
    }

    // ── Singleton ─────────────────────────────────────────────────────────────

    void EditorExtensionsSettingsWindow::ShowOrRaise()
    {
        if (!s_instance)
            s_instance = new EditorExtensionsSettingsWindow(nullptr);

        s_instance->show();
        s_instance->raise();
        s_instance->activateWindow();
    }

    void EditorExtensionsSettingsWindow::CloseIfOpen()
    {
        if (s_instance)
            s_instance->close();
        // close() triggers closeEvent → s_instance = nullptr; deleteLater()
    }

    void EditorExtensionsSettingsWindow::closeEvent(QCloseEvent* event)
    {
        SaveState();
        QMainWindow::closeEvent(event);
        s_instance = nullptr;
        deleteLater();
    }

    // ── Persistence ───────────────────────────────────────────────────────────

    void EditorExtensionsSettingsWindow::SaveState()
    {
        QSettings settings;
        settings.beginGroup("HeathenEditor/SettingsWindow");
        settings.setValue("Geometry", saveGeometry());
        settings.setValue("SplitterState", m_splitter->saveState());
        // Save the currently selected nav page path
        QTreeWidgetItem* cur = m_navTree->currentItem();
        if (cur)
            settings.setValue("LastPage", cur->data(0, Qt::UserRole).toString());
        settings.endGroup();
    }

    void EditorExtensionsSettingsWindow::RestoreState()
    {
        QSettings settings;
        settings.beginGroup("HeathenEditor/SettingsWindow");

        const QByteArray geometry = settings.value("Geometry").toByteArray();
        if (!geometry.isEmpty())
            restoreGeometry(geometry);

        const QByteArray splitterState = settings.value("SplitterState").toByteArray();
        if (!splitterState.isEmpty())
            m_splitter->restoreState(splitterState);

        // Re-select the last page
        const QString lastPage = settings.value("LastPage").toString();
        settings.endGroup();

        if (!lastPage.isEmpty())
        {
            QTreeWidgetItemIterator it(m_navTree);
            while (*it)
            {
                if ((*it)->data(0, Qt::UserRole).toString() == lastPage)
                {
                    m_navTree->setCurrentItem(*it);
                    return;
                }
                ++it;
            }
        }

        // Default: select first leaf
        QTreeWidgetItemIterator first(m_navTree, QTreeWidgetItemIterator::NoChildren);
        if (*first)
            m_navTree->setCurrentItem(*first);
    }

    // ── Nav / page construction ───────────────────────────────────────────────

    void EditorExtensionsSettingsWindow::BuildNavAndPages()
    {
        auto* registry = AZ::Interface<ISettingsPageRegistry>::Get();
        if (!registry)
            return;

        const auto& pages = registry->GetPages();

        // Group nodes by path — build the QTreeWidget hierarchy.
        // "Player/Steamworks" creates: Player (group) → Steamworks (leaf)
        // Top-level items with no "/" are just leaves directly under the root.
        QMap<QString, QTreeWidgetItem*> groupItems; // "Player" → QTreeWidgetItem*

        for (int i = 0; i < static_cast<int>(pages.size()); ++i)
        {
            const AZStd::string& azPath = pages[i]->GetPath(); // raw ptr — registry owns lifetime
            const QString path = QString::fromUtf8(azPath.c_str());
            const QStringList parts = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);

            if (parts.isEmpty())
                continue;

            QTreeWidgetItem* parent = nullptr;

            // Create/reuse group nodes for all but the last segment
            QString builtPath;
            for (int p = 0; p < parts.size() - 1; ++p)
            {
                if (!builtPath.isEmpty())
                    builtPath += QLatin1Char('/');
                builtPath += parts[p];

                if (!groupItems.contains(builtPath))
                {
                    auto* group = parent
                        ? new QTreeWidgetItem(parent)
                        : new QTreeWidgetItem(m_navTree);
                    group->setText(0, parts[p]);
                    group->setFlags(group->flags() & ~Qt::ItemIsSelectable);
                    group->setExpanded(true);
                    groupItems[builtPath] = group;
                }
                parent = groupItems[builtPath];
            }

            // Create the leaf node
            auto* leaf = parent
                ? new QTreeWidgetItem(parent)
                : new QTreeWidgetItem(m_navTree);
            leaf->setText(0, parts.last());
            leaf->setData(0, Qt::UserRole, path); // full path for page lookup

            m_pathToStackIndex[path] = i;

            // Add a placeholder to the stack now; the real widget is created lazily on first show.
            m_pageStack->addWidget(new QWidget(m_pageStack));
            m_pageCreated.push_back(false);
        }

        m_navTree->expandAll();
    }

    void EditorExtensionsSettingsWindow::EnsurePageCreated(int stackIndex)
    {
        if (stackIndex < 0 || stackIndex >= m_pageCreated.size())
            return;
        if (m_pageCreated[stackIndex])
            return;

        auto* registry = AZ::Interface<ISettingsPageRegistry>::Get();
        if (!registry)
            return;

        const AZStd::vector<ISettingsPage*>& pages = registry->GetPages();
        if (stackIndex >= static_cast<int>(pages.size()))
            return;

        QWidget* pageWidget = pages[stackIndex]->CreateWidget(m_pageStack);
        if (!pageWidget)
            return;

        // Replace the placeholder at this index
        QWidget* placeholder = m_pageStack->widget(stackIndex);
        m_pageStack->insertWidget(stackIndex, pageWidget);
        m_pageStack->removeWidget(placeholder);
        delete placeholder;

        m_pageCreated[stackIndex] = true;
    }

    // ── Slots ─────────────────────────────────────────────────────────────────

    void EditorExtensionsSettingsWindow::OnNavSelectionChanged(QTreeWidgetItem* current,
                                                                QTreeWidgetItem* /*previous*/)
    {
        if (!current)
            return;

        const QString path = current->data(0, Qt::UserRole).toString();
        if (path.isEmpty())
            return; // group node — not selectable

        const int idx = m_pathToStackIndex.value(path, -1);
        if (idx < 0)
            return;

        EnsurePageCreated(idx);
        m_pageStack->setCurrentIndex(idx);

        // Persist the selected page immediately so it survives a crash/close
        QSettings settings;
        settings.setValue("HeathenEditor/SettingsWindow/LastPage", path);
    }

    void EditorExtensionsSettingsWindow::OnSearchTextChanged(const QString& text)
    {
        ApplySearchFilter(text);
    }

    void EditorExtensionsSettingsWindow::ApplySearchFilter(const QString& filter)
    {
        const QString f = filter.trimmed().toLower();

        QTreeWidgetItemIterator it(m_navTree);
        while (*it)
        {
            QTreeWidgetItem* item = *it;
            const bool isLeaf     = (item->childCount() == 0);
            const bool matches    = f.isEmpty()
                || item->text(0).toLower().contains(f);

            if (isLeaf)
                item->setHidden(!matches);

            ++it;
        }

        // Show group nodes only when at least one of their descendants is visible
        QTreeWidgetItemIterator git(m_navTree, QTreeWidgetItemIterator::HasChildren);
        while (*git)
        {
            QTreeWidgetItem* group = *git;
            bool anyVisible = false;
            for (int c = 0; c < group->childCount(); ++c)
            {
                if (!group->child(c)->isHidden())
                {
                    anyVisible = true;
                    break;
                }
            }
            group->setHidden(!anyVisible);
            ++git;
        }
    }

} // namespace EditorExtensions

#include <moc_EditorExtensionsSettingsWindow.cpp>
