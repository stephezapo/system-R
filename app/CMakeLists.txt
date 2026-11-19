# -----------------------------------------------------------------------------
# system-R — the JUCE GUI application.
# -----------------------------------------------------------------------------
juce_add_gui_app(systemr_app
    PRODUCT_NAME "system-R"
    COMPANY_NAME "system-R"
    VERSION      "${PROJECT_VERSION}"
    BUNDLE_ID    "org.stephezapo.systemr")

target_sources(systemr_app PRIVATE
    src/Main.cpp)

# Embedded resources (exposed as BinaryData::*). Lucide icon font: ISC licence,
# see resources/fonts/LICENSE-lucide.txt.
juce_add_binary_data(systemr_app_resources
    SOURCES resources/fonts/lucide.ttf)

# Trim JUCE features we do not use yet (keeps the build lean and dependency-free
# on Linux: no libcurl / webkit needed).
target_compile_definitions(systemr_app PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:systemr_app,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:systemr_app,JUCE_VERSION>")

target_link_libraries(systemr_app PRIVATE
    systemr::core
    systemr_app_resources
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
    juce::juce_recommended_lto_flags)
