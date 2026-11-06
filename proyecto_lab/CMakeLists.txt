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

# Shader/images/Models como symlinks: editas ahí y no hace falta re-copiar
foreach(RES Shader images Models)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${RES})
        file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/${RES}
             ${CMAKE_CURRENT_BINARY_DIR}/${RES} SYMBOLIC)
    endif()
endforeach()