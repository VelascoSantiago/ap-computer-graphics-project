get_filename_component(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(SOURCE ${SOURCES})
    get_filename_component(SOURCE_NAME ${SOURCE} NAME_WE)
    set(T ${TARGET_NAME}_${SOURCE_NAME})
    add_executable(${T} ${SOURCE})
    target_include_directories(${T} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/dependencias
        ${CMAKE_SOURCE_DIR}/dependencias/glm
    )
    target_link_libraries(${T} PRIVATE OpenGL::GL glfw glad soil2 assimp::assimp)
endforeach()

if(MSVC)
    target_compile_definitions(${T} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX _USE_MATH_DEFINES)
endif()

# Recursos: en Linux, symlinks (editas y no hace falta re-copiar); en Windows los symlinks
# requieren permisos, asi que se copian junto al ejecutable despues de compilar.
foreach(RES Shader images Models UI)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${RES})
        if(WIN32)
            add_custom_command(TARGET ${T} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_CURRENT_SOURCE_DIR}/${RES} $<TARGET_FILE_DIR:${T}>/${RES})
        else()
            file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/${RES}
                 ${CMAKE_CURRENT_BINARY_DIR}/${RES} SYMBOLIC)
        endif()
    endif()
endforeach()
