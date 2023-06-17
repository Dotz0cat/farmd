project('farmd', 'c', version: run_command('python', 'get_version.py', check: true).stdout().strip())
cc = meson.get_compiler('c')
src = ['src/main.c', 'src/loop.c', 'src/save.c', 'src/list.c', 'src/inital_values.c', 'src/config.c',
'src/barn.c', 'src/silo.c', 'src/tree.c', 'src/field.c', 'src/econ.c', 'src/skill.c', 'src/meta.c', 'src/storage.c',
'src/grain_mill.c']
deps = [dependency('libconfig'), dependency('libevent_openssl'), dependency('sqlite3'), dependency('openssl'), cc.find_library('m', required : false)]
executable('farmd', src, dependencies: deps, c_args: '-DVERSION="@0@"'.format(meson.project_version()), install: true)
