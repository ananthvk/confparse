gtest_dep = dependency('gtest')

cppstarter_test = executable(
    'cppstarter_test',
    sources: ['cppstarter_test.cpp'],
    dependencies: [gtest_dep],
    include_directories: include_dirs,
    cpp_args: extra_args,
)
test('cppstarter_test', cppstarter_test)