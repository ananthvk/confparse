gtest_dep = dependency('gtest')

confparse_test = executable(
    'confparse_test',
    sources: ['confparse_test.cpp'],
    dependencies: [gtest_dep],
    include_directories: include_dirs,
    cpp_args: extra_args,
)
test('confparse_test', confparse_test)