
#compiller flags
if( NOT DISABLE_DEBUG )
    set( CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} -D__DAVAENGINE_DEBUG__" )

endif  ()

if     ( ANDROID )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -mfloat-abi=softfp -mfpu=neon -frtti" )    
    set( CMAKE_ECLIPSE_MAKE_ARGUMENTS -j8 )
    
elseif ( IOS     ) 
    set( CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS} -O0" )
    set( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} -O3" )

    set( CMAKE_XCODE_ATTRIBUTE_OTHER_LDFLAGS "-ObjC" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )
    set( CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY iPhone/iPad )
    set( CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 7.0 )
    set( CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE No )
    set( CMAKE_EXE_LINKER_FLAGS "-ObjC" )

    set( CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)" )

    if( NOT CMAKE_IOS_SDK_ROOT )
        set( CMAKE_IOS_SDK_ROOT Latest IOS )

    endif()

    if( NOT IOS_BUNDLE_IDENTIFIER )
        set( IOS_BUNDLE_IDENTIFIER com.davaconsulting.${PROJECT_NAME} )
        
    endif()
    
    # Fix try_compile
    set( MACOSX_BUNDLE_GUI_IDENTIFIER  ${IOS_BUNDLE_IDENTIFIER} )
    set( CMAKE_MACOSX_BUNDLE YES )
    set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" )

    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES} AdHoc" CACHE STRING
        "Semicolon separated list of supported configuration types [Debug|Release|AdHoc]"
        FORCE)
     
    set(CMAKE_C_FLAGS_ADHOC             ${CMAKE_C_FLAGS_RELEASE} )
    set(CMAKE_CXX_FLAGS_ADHOC           ${CMAKE_CXX_FLAGS_RELEASE} )
    set(CMAKE_EXE_LINKER_FLAGS_ADHOC    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} )
    set(CMAKE_SHARED_LINKER_FLAGS_ADHOC ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} )
    set(CMAKE_MODULE_LINKER_FLAGS_ADHOC ${CMAKE_MODULE_LINKER_FLAGS_RELEASE} )
     
    mark_as_advanced(   CMAKE_C_FLAGS_ADHOC 
                        CMAKE_CXX_FLAGS_ADHOC
                        CMAKE_EXE_LINKER_FLAGS_ADHOC 
                        CMAKE_SHARED_LINKER_FLAGS_ADHOC 
                        CMAKE_MODULE_LINKER_FLAGS_ADHOC  )

elseif ( MACOS )
    set( CMAKE_OSX_DEPLOYMENT_TARGET "" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )
    set( CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS YES )
    set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )

elseif ( WIN32 )
    #dynamic runtime on windows store
    if ( WINDOWS_UAP )
        set ( CRT_TYPE_DEBUG "/MDd" )
        set ( CRT_TYPE_RELEASE "/MD" )
        #consume windows runtime extension (C++/CX)
        set ( ADDITIONAL_CXX_FLAGS "/ZW")
    else ()
        set ( CRT_TYPE_DEBUG "/MTd" )
        set ( CRT_TYPE_RELEASE "/MT" )
    endif ()
    
    # ignorance of linker warnings
    set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4099,4221,4264" )
    set ( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /IGNORE:4099,4221,4264" )
    set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /IGNORE:4099" )
    if ( NOT WINDOWS_UAP )
        set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /delayload:d3dcompiler_47.dll" )
    endif ()

    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CRT_TYPE_DEBUG} ${ADDITIONAL_CXX_FLAGS} /MP /EHsc /Zi /Od /bigobj" ) 
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CRT_TYPE_RELEASE} ${ADDITIONAL_CXX_FLAGS} /MP /EHsc /bigobj" ) 
    set ( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} /Zi" )
    set ( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /ENTRY:mainCRTStartup /INCREMENTAL:NO" )
    set ( CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG" )

    if ( DEBUG_INFO )
        set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi" ) 
    endif ()

    # undef macros min and max defined in windows.h
    add_definitions ( -DNOMINMAX )
endif  ()


##
if( WARNING_DISABLE)

    if( WIN32 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0" )
    elseif( APPLE )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w" )
    endif()


elseif( WARNINGS_AS_ERRORS )


    set(LOCAL_DISABLED_WARNINGS "-Weverything \
-Werror \
-Wno-c++98-compat-pedantic \
-Wno-newline-eof \
-Wno-gnu-anonymous-struct \
-Wno-nested-anon-types \
-Wno-float-equal \
-Wno-extra-semi \
-Wno-unused-parameter \
-Wno-shadow \
-Wno-exit-time-destructors \
-Wno-documentation \
-Wno-global-constructors \
-Wno-padded \
-Wno-weak-vtables \
-Wno-variadic-macros \
-Wno-deprecated-register \
-Wno-sign-conversion \
-Wno-sign-compare \
-Wno-format-nonliteral \
-Wno-cast-align \
-Wno-conversion \
-Wno-unreachable-code \
-Wno-zero-length-array \
-Wno-switch-enum \
-Wno-c99-extensions \
-Wno-missing-prototypes \
-Wno-missing-field-initializers \
-Wno-conditional-uninitialized \
-Wno-covered-switch-default \
-Wno-deprecated \
-Wno-unused-macros \
-Wno-disabled-macro-expansion \
-Wno-undef \
-Wno-non-virtual-dtor \
-Wno-char-subscripts \
-Wno-unneeded-internal-declaration \
-Wno-unused-variable \
-Wno-used-but-marked-unused \
-Wno-missing-variable-declarations \
-Wno-gnu-statement-expression \
-Wno-missing-braces \
-Wno-reorder \
-Wno-implicit-fallthrough \
-Wno-ignored-qualifiers \
-Wno-shift-sign-overflow \
-Wno-mismatched-tags \
-Wno-missing-noreturn \
-Wno-consumed \
-Wno-sometimes-uninitialized \
-Wno-delete-non-virtual-dtor \
-Wno-header-hygiene \
-Wno-old-style-cast \
-Wno-unknown-warning-option \
-Wno-unreachable-code-return \
-Wno-unreachable-code-break \
-Wno-reserved-id-macro \
-Wno-documentation-pedantic \
-Wno-inconsistent-missing-override \
-Wno-unused-local-typedef \
-Wno-nullable-to-nonnull-conversion \
-Wno-super-class-method-mismatch \
-Wno-nonnull")


    if( ANDROID )
        set( LOCAL_DISABLED_WARNINGS "${LOCAL_DISABLED_WARNINGS} \
-Wno-reserved-id-macro \
-Wno-unused-local-typedef \
-Wno-inconsistent-missing-override \
-Wno-unknown-pragmas")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCAL_DISABLED_WARNINGS}" ) # warnings as errors
    elseif( APPLE )
        set( LOCAL_DISABLED_WARNINGS "${LOCAL_DISABLED_WARNINGS} \
-Wno-cstring-format-directive \
-Wno-duplicate-enum \
-Wno-infinite-recursion \
-Wno-objc-interface-ivars \
-Wno-direct-ivar-access \
-Wno-objc-missing-property-synthesis \
-Wno-over-aligned \
-Wno-unused-exception-parameter \
-Wno-idiomatic-parentheses \
-Wno-vla-extension \
-Wno-vla \
-Wno-overriding-method-mismatch \
-Wno-method-signatures \
-Wno-receiver-forward-class \
-Wno-semicolon-before-method-body \
-Wno-import-preprocessor-directive-pedantic" )

        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCAL_DISABLED_WARNINGS}" ) # warnings as errors
    elseif( WIN32 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX" )
    endif()

endif()


##
if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/ios" ) 
  
elseif ( MACOS )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/mac" ) 

elseif ( WIN32)
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win" ) 
    
endif  ()
