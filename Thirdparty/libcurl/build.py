import os
import shutil
import build_utils


def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10']
    else:
        return ['macos', 'ios', 'android']


def get_dependencies_for_target(target):
    if target == 'android':
        return ['openssl']
    else:
        return []


def build_for_target(target, working_directory_path, root_project_path):
    if target == 'win32':
        _build_win32(working_directory_path, root_project_path)
    elif target == 'win10':
        _build_win10(working_directory_path, root_project_path)
    elif target == 'macos':
        _build_macos(working_directory_path, root_project_path)
    elif target == 'ios':
        _build_ios(working_directory_path, root_project_path)
    elif target == 'android':
        _build_android(working_directory_path, root_project_path)


def get_download_info():
    return {'macos_and_ios': 'maintained by curl-ios-build-scripts (bundled)',
            'others': 'https://curl.haxx.se/download/curl-7.50.3.tar.gz'}


def _download_and_extract(working_directory_path):
    source_folder_path = os.path.join(working_directory_path, 'libcurl_source')

    url = get_download_info()['others']
    build_utils.download_and_extract(
        url,
        working_directory_path,
        source_folder_path,
        build_utils.get_url_file_name_no_ext(url))

    return source_folder_path


@build_utils.run_once
def _patch_sources(source_folder_path, working_directory_path):
    # Apply fixes
    build_utils.apply_patch(
        os.path.abspath('patch.diff'), working_directory_path)


def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    vc12_solution_file_path = os.path.join(
        source_folder_path, 'projects/Windows/VC12/curl-all.sln')
    build_utils.build_vs(
        vc12_solution_file_path,
        'LIB Debug - DLL Windows SSPI', 'Win32', 'libcurl')
    build_utils.build_vs(
        vc12_solution_file_path,
        'LIB Release - DLL Windows SSPI', 'Win32', 'libcurl')
    build_utils.build_vs(
        vc12_solution_file_path,
        'LIB Debug - DLL Windows SSPI', 'x64', 'libcurl')
    build_utils.build_vs(
        vc12_solution_file_path,
        'LIB Release - DLL Windows SSPI', 'x64', 'libcurl')

    libs_win_root = os.path.join(root_project_path, 'Libs/lib_CMake/win')

    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win32/VC12/LIB Debug - DLL Windows SSPI/libcurld.lib'),
        os.path.join(libs_win_root, 'x86/Debug/libcurl.lib'))
    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win32/VC12/LIB Release - DLL Windows SSPI/libcurl.lib'),
        os.path.join(libs_win_root, 'x86/Release/libcurl.lib'))
    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win64/VC12/LIB Debug - DLL Windows SSPI/libcurld.lib'),
        os.path.join(libs_win_root, 'x64/Debug/libcurl_a_debug.lib'))
    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win64/VC12/LIB Release - DLL Windows SSPI/libcurl.lib'),
        os.path.join(libs_win_root, 'x64/Release/libcurl_a.lib'))

    _copy_headers(source_folder_path, root_project_path, 'Others')


def _build_win10(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)

    vc14_solution_folder_path = os.path.join(
        source_folder_path, 'projects/Windows/VC14')
    vc14_solution_file_path = os.path.join(
        vc14_solution_folder_path, 'curl-all.sln')

    build_utils.build_vs(
        vc14_solution_file_path,
        'LIB Debug - DLL Windows SSPI', 'Win32', 'libcurl')
    build_utils.build_vs(
        vc14_solution_file_path,
        'LIB Release - DLL Windows SSPI', 'Win32', 'libcurl')
    build_utils.build_vs(
        vc14_solution_file_path,
        'LIB Debug - DLL Windows SSPI', 'x64', 'libcurl')
    build_utils.build_vs(
        vc14_solution_file_path,
        'LIB Release - DLL Windows SSPI', 'x64', 'libcurl')
    build_utils.build_vs(
        vc14_solution_file_path,
        'LIB Debug - DLL Windows SSPI', 'ARM', 'libcurl')
    build_utils.build_vs(
        vc14_solution_file_path,
        'LIB Release - DLL Windows SSPI', 'ARM', 'libcurl')

    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win32/VC14/LIB Debug - DLL Windows SSPI/libcurld.lib'),
        os.path.join(
            root_project_path,
            'Libs/lib_CMake/win10/Win32/Debug/libcurl.lib'))
    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win32/VC14/LIB Release - DLL Windows SSPI/libcurl.lib'),
        os.path.join(
            root_project_path,
            'Libs/lib_CMake/win10/Win32/Release/libcurl.lib'))
    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win64/VC14/LIB Debug - DLL Windows SSPI/libcurld.lib'),
        os.path.join(
            root_project_path,
            'Libs/lib_CMake/win10/x64/Debug/libcurl.lib'))
    shutil.copyfile(
        os.path.join(
            source_folder_path,
            'build/Win64/VC14/LIB Release - DLL Windows SSPI/libcurl.lib'),
        os.path.join(
            root_project_path, 'Libs/lib_CMake/win10/x64/Release/libcurl.lib'))

    # ARM outptu folder isn't specifically set by solution, so it's a default one

    shutil.copyfile(
        os.path.join(
            vc14_solution_folder_path,
            'ARM/LIB Debug - DLL Windows SSPI/libcurld.lib'),
        os.path.join(
            root_project_path,
            'Libs/lib_CMake/win10/arm/Debug/libcurl.lib'))
    shutil.copyfile(
        os.path.join(
            vc14_solution_folder_path,
            'ARM/LIB Release - DLL Windows SSPI/libcurl.lib'),
        os.path.join(
            root_project_path,
            'Libs/lib_CMake/win10/arm/Release/libcurl.lib'))

    _copy_headers(source_folder_path, root_project_path, 'Others')


def _build_macos(working_directory_path, root_project_path):
    build_curl_run_dir = os.path.join(working_directory_path, 'gen/build_osx')

    if not os.path.exists(build_curl_run_dir):
        os.makedirs(build_curl_run_dir)

    build_curl_args = [
        './build_curl', '--arch', 'x86_64', '--run-dir', build_curl_run_dir]
    if (build_utils.verbose):
        build_curl_args.append('--verbose')

    build_utils.run_process(
        build_curl_args,
        process_cwd='curl-ios-build-scripts-master')

    output_path = os.path.join(build_curl_run_dir, 'curl/osx/lib/libcurl.a')

    shutil.copyfile(
        output_path,
        os.path.join(
            root_project_path,
            os.path.join('Libs/lib_CMake/mac/libcurl_macos.a')))

    include_path = os.path.join(
        root_project_path,
        os.path.join('Libs/include/curl/iOS_MacOS'))
    build_utils.copy_files(
        os.path.join(build_curl_run_dir, 'curl/osx/include'),
        include_path,
        '*.h')


def _build_ios(working_directory_path, root_project_path):
    build_curl_run_dir = os.path.join(working_directory_path, 'gen/build_ios')

    if not os.path.exists(build_curl_run_dir):
        os.makedirs(build_curl_run_dir)

    build_curl_args = [
        './build_curl',
        '--arch',
        'armv7,armv7s,arm64',
        '--run-dir',
        build_curl_run_dir]
    if (build_utils.verbose):
        build_curl_args.append('--verbose')

    build_utils.run_process(
        build_curl_args, process_cwd='curl-ios-build-scripts-master')

    output_path = os.path.join(
        build_curl_run_dir, 'curl/ios-appstore/lib/libcurl.a')

    shutil.copyfile(
        output_path,
        os.path.join(
            root_project_path,
            os.path.join('Libs/lib_CMake/ios/libcurl_ios.a')))

    include_path = os.path.join(
        root_project_path, os.path.join('Libs/include/curl/iOS_MacOS'))
    build_utils.copy_files(
        os.path.join(build_curl_run_dir, 'curl/ios-appstore/include'),
        include_path,
        '*.h')


def _build_android(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    env = os.environ.copy()
    original_path_var = env["PATH"]

    # ARM

    toolchain_path_arm = os.path.join(
        working_directory_path, 'gen/ndk_toolchain_arm')
    build_utils.android_ndk_make_toolchain(
        root_project_path,
        'arm',
        'android-14',
        'darwin-x86_64',
        toolchain_path_arm)

    env['PATH'] = '{}:{}'.format(
        os.path.join(toolchain_path_arm, 'bin'), original_path_var)
    install_dir_arm = os.path.join(working_directory_path, 'gen/install_arm')
    configure_args = [
        '--host=arm-linux-androideabi',
        '--disable-shared',
        '--with-ssl=' + os.path.abspath(
            os.path.join(
                working_directory_path, '../openssl/gen/install_arm/'))]
    build_utils.build_with_autotools(
        source_folder_path,
        configure_args,
        install_dir_arm,
        env)

    # x86

    toolchain_path_x86 = os.path.join(
        working_directory_path, 'gen/ndk_toolchain_x86')
    build_utils.android_ndk_make_toolchain(
        root_project_path,
        'x86',
        'android-14',
        'darwin-x86_64',
        toolchain_path_x86)

    env['PATH'] = '{}:{}'.format(
        os.path.join(toolchain_path_x86, 'bin'), original_path_var)
    install_dir_arm = os.path.join(working_directory_path, 'gen/install_x86')
    configure_args = [
        '--host=i686-linux-android',
        '--disable-shared',
        '--with-ssl=' + os.path.abspath(
            os.path.join(
                working_directory_path,
                '../openssl/gen/install_x86/'))]
    build_utils.build_with_autotools(
        source_folder_path,
        configure_args,
        install_dir_arm, env)

    _copy_headers(source_folder_path, root_project_path, 'Others')


def _copy_headers(source_folder_path, root_project_path, target_folder):
    include_path = os.path.join(
        root_project_path, os.path.join('Libs/include/curl', target_folder))
    build_utils.copy_files(
        os.path.join(source_folder_path, 'include/curl'), include_path, '*.h')
