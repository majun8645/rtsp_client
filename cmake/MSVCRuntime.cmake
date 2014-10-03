
macro(configure_msvc_runtime)
	if(MSVC)
		# Default to statically-linked runtime.
		if("${MSVC_RUNTIME}" STREQUAL "")
			set(MSVC_RUNTIME "static")
		endif()

	#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE -D_UNICODE")
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WIN32_WINNT=0x0501")
	#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWIN32_LEAN_AND_MEAN")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  #set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} /INCREMENTAL:NO /SAFESEH:NO")
		add_definitions(-D_UNICODE -DUNICODE -D_CRT_SECURE_NO_WARNINGS)

		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO /SAFESEH:NO")

		# Set compiler options.
			#CMAKE_C_FLAGS_MINSIZEREL
			#CMAKE_C_FLAGS_RELWITHDEBINFO
			#CMAKE_CXX_FLAGS_MINSIZEREL
			#CMAKE_CXX_FLAGS_RELWITHDEBINFO
		set(variables
			CMAKE_C_FLAGS
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE
    )

		if(${MSVC_RUNTIME} STREQUAL "static")
			message(STATUS "MSVC: using statically-linked runtime (/MT and /MTd).")
			foreach(variable ${variables})
				if(${variable} MATCHES "/MD")
					message(STATUS "${variable} before ${${variable}}")
					string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
					set(${variable} "${${variable}} /MP")
					message(STATUS "${variable} after ${${variable}}")
				endif()
			endforeach()
		else()
			message(STATUS "MSVC: using dynamically-linked runtime (/MD and /MDd).")
			foreach(variable ${variables})
				if(${variable} MATCHES "/MT")
					string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
				endif()
			endforeach()
		endif()

		foreach(variable ${variables})
			if(${variable} MATCHES "/Ob0")
				string(REGEX REPLACE "/Ob0" "/Ob2" ${variable} "${${variable}}")
			endif()
			if(${variable} MATCHES "/W3")
				string(REGEX REPLACE "/W3" "/W2" ${variable} "${${variable}}")
			endif()
		endforeach()

		#foreach(variable ${variables})
			#if(${variable} MATCHES "/W3")
				#string(REGEX REPLACE "/W3" "/W2" ${variable} "${${variable}}")
			#endif()
		#endforeach()

		set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
		#set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
		#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")

		foreach(variable ${variables})
			set(${variable} "${${variable}}" CACHE STRING "MSVC_${variable}" FORCE)
		endforeach()
	endif()
endmacro(configure_msvc_runtime)
