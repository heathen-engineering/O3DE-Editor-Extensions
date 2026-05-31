
o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

if(NOT PAL_TRAIT_EDITOREXTENSIONS_SUPPORTED)
    return()
endif()

# ── Runtime stub ──────────────────────────────────────────────────────────────
# EditorExtensions has no runtime functionality. The stub module satisfies the
# O3DE gem activation system's requirement for Clients/Servers/Unified targets.

ly_add_target(
    NAME ${gem_name} ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        editorextensions_shared_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC  Include
        PRIVATE Source
    BUILD_DEPENDENCIES
        PUBLIC AZ::AzCore
)

ly_add_source_properties(
    SOURCES Source/Clients/EditorExtensionsModule.cpp
    PROPERTY COMPILE_DEFINITIONS
    VALUES
        O3DE_GEM_NAME=${gem_name}
        O3DE_GEM_VERSION=${gem_version})

ly_create_alias(NAME ${gem_name}.Clients  NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Servers  NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Unified  NAMESPACE Gem TARGETS Gem::${gem_name})

ly_create_alias(NAME ${gem_name}.Clients.API NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Servers.API NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Unified.API NAMESPACE Gem TARGETS Gem::${gem_name})

o3de_add_variant_dependencies_for_gem_dependencies(GEM_NAME ${gem_name} VARIANTS Clients Servers Unified)

# ── Editor targets ────────────────────────────────────────────────────────────
if(PAL_TRAIT_BUILD_HOST_TOOLS)

    # Public API: ISettingsPage, ISettingsPageRegistry, TypeIds headers.
    # Other gems' editor targets depend on this to register settings pages.
    ly_add_target(
        NAME ${gem_name}.Editor.API INTERFACE
        NAMESPACE Gem
        FILES_CMAKE
            editorextensions_editor_api_files.cmake
        INCLUDE_DIRECTORIES
            INTERFACE Include
        BUILD_DEPENDENCIES
            INTERFACE AZ::AzToolsFramework
    )

    # Private implementation: SettingsWindow, EditorComponent, Qt widgets.
    ly_add_target(
        NAME ${gem_name}.Editor.Private.Object STATIC
        NAMESPACE Gem
        AUTOMOC
        AUTORCC
        FILES_CMAKE
            editorextensions_editor_private_files.cmake
        TARGET_PROPERTIES
            O3DE_PRIVATE_TARGET TRUE
        INCLUDE_DIRECTORIES
            PRIVATE Include Source
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                Gem::${gem_name}.Editor.API
    )

    # Editor GEM_MODULE: bootstraps the editor component.
    ly_add_target(
        NAME ${gem_name}.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            editorextensions_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PUBLIC  Include
            PRIVATE Source
        BUILD_DEPENDENCIES
            PUBLIC  Gem::${gem_name}.Editor.API
            PRIVATE Gem::${gem_name}.Editor.Private.Object
    )

    ly_add_source_properties(
        SOURCES Source/Tools/EditorExtensionsEditorModule.cpp
        PROPERTY COMPILE_DEFINITIONS
        VALUES
            O3DE_GEM_NAME=${gem_name}
            O3DE_GEM_VERSION=${gem_version})

    ly_create_alias(NAME ${gem_name}.Tools    NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)
    ly_create_alias(NAME ${gem_name}.Builders NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)

    ly_create_alias(NAME ${gem_name}.Tools.API    NAMESPACE Gem TARGETS Gem::${gem_name}.Editor.API)
    ly_create_alias(NAME ${gem_name}.Builders.API NAMESPACE Gem TARGETS Gem::${gem_name}.Editor.API)

    o3de_add_variant_dependencies_for_gem_dependencies(GEM_NAME ${gem_name} VARIANTS Tools Builders)

endif()
