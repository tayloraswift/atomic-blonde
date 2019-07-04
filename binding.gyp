{
    'targets': [{
        'target_name': 'blonde',
        'include_dirs': [
            '<!(node -e \"require(\'nan\')\")'
        ],
        'sources': [
            'cpp/blonde.cpp'
        ],
        'compiler_checks': [
            '-Wall',
            '-Wextra',
            '-Weffc++',
            '-Wconversion',
            '-pedantic-errors',
            '-Wconversion',
            '-Wshadow',
            '-Wfloat-equal',
            '-Wuninitialized',
            '-Wunreachable-code',
            '-Wold-style-cast',
            '-Wno-error=unused-variable'
        ],
        'libraries': []
    }]
}
