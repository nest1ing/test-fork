project('test-fork', 'cpp',
  default_options: [
    'warning_level=3',
    'werror=true',
    'cpp_std=c++17',
  ],
  license: 'Apache-2.0',
  version: '0.1',
)

sdeventplus = dependency('sdeventplus')

executable('test-fork-parent',
  [
    'src/parent.cpp',
  ],
  dependencies: [
    sdeventplus,
  ],
)

executable('test-fork-child',
  [
    'src/child.cpp',
  ],
  dependencies: [
    sdeventplus,
  ],
)
