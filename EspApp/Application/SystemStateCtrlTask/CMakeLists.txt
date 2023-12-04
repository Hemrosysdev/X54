set(COMPONENT_SRCDIRS Sources YakinduImpl YakinduGenCode )
set(COMPONENT_ADD_INCLUDEDIRS Interfaces YakinduLibrary YakinduImpl YakinduGenCode )

set( COMPONENT_REQUIRES UxEspCppLibrary Main Common mdns )

register_component()
