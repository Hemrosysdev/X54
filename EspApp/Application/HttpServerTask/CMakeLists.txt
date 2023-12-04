set(COMPONENT_SRCDIRS Sources )
set(COMPONENT_ADD_INCLUDEDIRS Interfaces )
set(COMPONENT_EMBED_FILES 	"${PROJECT_DIR}/build/index.html.stripped" 
                            "${PROJECT_DIR}/build/wifi.html.stripped" 
                            "${PROJECT_DIR}/build/login.html.stripped" 
							"${PROJECT_DIR}/build/copyright.html.stripped" 
                            "${PROJECT_DIR}/build/x54_websocket_test.html.stripped" 
							"${PROJECT_DIR}/build/favicon.ico.gz" 
							"${PROJECT_DIR}/build/zepto.min.js.gz"
							"${PROJECT_DIR}/build/style.css.stripped.gz"
							"WebSite/x54.jpg" 
                            "WebSite/wifi.png" 
                            "WebSite/home.png" 
							"WebSite/mahlkoenig_logo_210x120.png"
							"WebSite/qrcode.png" )

set( COMPONENT_REQUIRES UxEspCppLibrary Main Common )

register_component()

set( MyPython "python" )

if(DEFINED ENV{windir})
set( MyPython "python.exe" )
endif()

# create index.html.stripped from index.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/index.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py index.html ${PROJECT_DIR}/build/index.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/index.html
    COMMENT "Generating index.html.stripped"
    VERBATIM )
add_custom_target( index_html_stripped DEPENDS ${PROJECT_DIR}/build/index.html.stripped )
add_dependencies( ${COMPONENT_LIB} index_html_stripped )

# create copyright.html.stripped from copyright.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/copyright.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py copyright.html ${PROJECT_DIR}/build/copyright.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/copyright.html
    COMMENT "Generating copyright.html.stripped"
    VERBATIM )
add_custom_target( copyright_html_stripped DEPENDS ${PROJECT_DIR}/build/copyright.html.stripped )
add_dependencies( ${COMPONENT_LIB} copyright_html_stripped )

# create wifi.html.stripped from wifi.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/wifi.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py wifi.html ${PROJECT_DIR}/build/wifi.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/wifi.html
    COMMENT "Generating wifi.html.stripped"
    VERBATIM )
add_custom_target( wifi_html_stripped DEPENDS ${PROJECT_DIR}/build/wifi.html.stripped )
add_dependencies( ${COMPONENT_LIB} wifi_html_stripped )

# create login.html.stripped from login.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/login.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py login.html ${PROJECT_DIR}/build/login.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/login.html
    COMMENT "Generating login.html.stripped"
    VERBATIM )
add_custom_target( login_html_stripped DEPENDS ${PROJECT_DIR}/build/login.html.stripped )
add_dependencies( ${COMPONENT_LIB} login_html_stripped )

# create x54_websocket_test.html.stripped from x54_websocket_test.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/x54_websocket_test.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py x54_websocket_test.html ${PROJECT_DIR}/build/x54_websocket_test.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/x54_websocket_test.html
    COMMENT "Generating x54_websocket_test.html.stripped"
    VERBATIM )
add_custom_target( x54_websocket_test_html_stripped DEPENDS ${PROJECT_DIR}/build/x54_websocket_test.html.stripped )
add_dependencies( ${COMPONENT_LIB} x54_websocket_test_html_stripped )

# create style.css.stripped.gz from style.css
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/style.css.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py style.css ${PROJECT_DIR}/build/style.css.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/style.css.stripped ${PROJECT_DIR}/build/style.css.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/style.css
    COMMENT "Generating style.css.stripped.gz"
    VERBATIM )
add_custom_target( style_css_stripped_gz DEPENDS ${PROJECT_DIR}/build/style.css.stripped.gz )
add_dependencies( ${COMPONENT_LIB} style_css_stripped_gz )

# create zepto.min.js.gz from zepto.min.js
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/zepto.min.js.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} gzip_helper.py zepto.min.js ${PROJECT_DIR}/build/zepto.min.js.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/zepto.min.js
    COMMENT "Generating zepto.min.js.gz"
    VERBATIM )
add_custom_target( zepto_min_js_gz DEPENDS ${PROJECT_DIR}/build/zepto.min.js.gz )
add_dependencies( ${COMPONENT_LIB} zepto_min_js_gz )

# create favicon.ico.gz from favicon.ico
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/favicon.ico.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} gzip_helper.py favicon.ico ${PROJECT_DIR}/build/favicon.ico.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/favicon.ico
    COMMENT "Generating favicon.ico.gz"
    VERBATIM )
add_custom_target( favicon_ico_gz DEPENDS ${PROJECT_DIR}/build/favicon.ico.gz )
add_dependencies( ${COMPONENT_LIB} favicon_ico_gz )
