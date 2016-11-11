include_directories (${SOURCE_DIR}/ ${THIRDPARTY_DIR}/boost/include/)

set (tool_VERSION "0.0.1")
set (tool_SRC
        model_tool.cpp
        model_tool.h)

add_executable(model_tool model_tool.cpp ../framework/multi_model.h)
target_link_libraries(model_tool boost_program_options_static_lib)
set_target_properties(model_tool PROPERTIES
        OUTPUT_NAME model_tool
        RUNTIME_OUTPUT_DIRECTORY ${TOOLS_DIR}/model_tool/)
