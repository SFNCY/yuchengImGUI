function(install_lib_func)
    cmake_parse_arguments(INSTALL_LIB "" "NAME;CMAKE_PATH;DESTINATION_MODULE_NAME;HEAD_DIR" "" ${ARGN})

    if(NOT INSTALL_LIB_DESTINATION_MODULE_NAME)
        set(INSTALL_LIB_DESTINATION_MODULE_NAME ${PROJECT_NAME})
    endif()

    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        ${PROJECT_BINARY_DIR}/${INSTALL_LIB_DESTINATION_MODULE_NAME}/${INSTALL_LIB_NAME}ConfigVersion.cmake
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(TARGETS ${INSTALL_LIB_NAME} 
            EXPORT ${INSTALL_LIB_NAME}Targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include
            )

    install(EXPORT ${INSTALL_LIB_NAME}Targets
            FILE ${INSTALL_LIB_NAME}Targets.cmake
            NAMESPACE ${INSTALL_LIB_DESTINATION_MODULE_NAME}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${INSTALL_LIB_DESTINATION_MODULE_NAME}
            )
    
    # install(DIRECTORY ${INSTALL_LIB_HEAD_DIR}
    #         DESTINATION ./include)

    configure_package_config_file(${INSTALL_LIB_CMAKE_PATH}/${INSTALL_LIB_NAME}Config.cmake.in
        "${PROJECT_BINARY_DIR}/${INSTALL_LIB_DESTINATION_MODULE_NAME}/${INSTALL_LIB_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${INSTALL_LIB_DESTINATION_MODULE_NAME})

    install(FILES "${PROJECT_BINARY_DIR}/${INSTALL_LIB_DESTINATION_MODULE_NAME}/${INSTALL_LIB_NAME}Config.cmake"
                    "${PROJECT_BINARY_DIR}/${INSTALL_LIB_DESTINATION_MODULE_NAME}/${INSTALL_LIB_NAME}ConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${INSTALL_LIB_DESTINATION_MODULE_NAME}
            )
endfunction()


function(install_lib_with_module)
    cmake_parse_arguments(INSTALL "" "LIB_NAME;MODULE_NAME;CMAKE_PATH" "" ${ARGN})

    if(NOT INSTALL_LIB_NAME)
        set(INSTALL_LIB_NAME ${PROJECT_NAME})
    endif()

    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # 生成版本文件，用于外部项目检查版本是否符合要求
    write_basic_package_version_file(
        ${PROJECT_BINARY_DIR}/${INSTALL_LIB_NAME}/${INSTALL_LIB_NAME}ConfigVersion.cmake
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    # 设置要安装的目标，指定文件夹
    install(TARGETS ${INSTALL_MODULE_NAME}
            EXPORT ${INSTALL_LIB_NAME}Targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/${INSTALL_MODULE_NAME}
            )

    # 安装导出文件，生成供外部项目查找和链接的目标文件
    install(EXPORT ${INSTALL_LIB_NAME}Targets
            FILE ${INSTALL_LIB_NAME}Targets.cmake
            NAMESPACE ${INSTALL_LIB_NAME}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${INSTALL_LIB_NAME}
            )

    # 从模板xxxConfig.cmake.in生成主配置文件xxxConfig.cmake，描述如何使用该库及如何找到相关依赖
    configure_package_config_file(${INSTALL_CMAKE_PATH}/${INSTALL_LIB_NAME}Config.cmake.in
        "${PROJECT_BINARY_DIR}/${INSTALL_LIB_NAME}/${INSTALL_LIB_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${INSTALL_LIB_NAME})

    # 安装配置文件，将xxxConfig.cmake与xxxConfigVersion.cmake安装到指定位置
    install(FILES "${PROJECT_BINARY_DIR}/${INSTALL_LIB_NAME}/${INSTALL_LIB_NAME}Config.cmake"
                    "${PROJECT_BINARY_DIR}/${INSTALL_LIB_NAME}/${INSTALL_LIB_NAME}ConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${INSTALL_LIB_NAME}
            )
endfunction()