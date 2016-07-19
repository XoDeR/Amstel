#
# project: fips-luajit
#
# Copyright (C) 2016 Volodymyr Syvochka

if (FIPS_WINDOWS AND FIPS_MSVC)
	# create a custom target which copies the precompiled
	# DLLs in lib/[os] to the executable output path
	set(luajit_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/lib/${FIPS_PLATFORM_NAME})
	set(luajit_dst_dir ${FIPS_DEPLOY_DIR}/${CMAKE_PROJECT_NAME}/${FIPS_CONFIG})
	add_custom_target(luajit_copy_dlls ALL
					  COMMAND cmake -E copy_directory ${luajit_src_dir} ${luajit_dst_dir}
					  COMMENT "Copy LuaJit DLLs...")
elseif (FIPS_LINUX)					  
# TODO					  
elseif (FIPS_ANDROID)
# TODO
elseif (FIPS_MACOS)
# TODO
elseif (FIPS_MACOS)
# TODO
elseif (FIPS_IOS)
# TODO
endif()