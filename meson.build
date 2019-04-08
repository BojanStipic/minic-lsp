project(
  'minic-lsp',
  'c',
  default_options : ['warning_level=3']
)

flex = find_program('flex')
bison = find_program('bison')

lgen = generator(
  flex,
  output : '@BASENAME@.yy.c',
  arguments : ['-o', '@OUTPUT@', '@INPUT@']
)
lfiles = lgen.process('minic.l')

pgen = generator(
  bison,
  output : ['@BASENAME@.tab.c', '@BASENAME@.tab.h'],
  arguments : ['@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@']
)
pfiles = pgen.process('minic.y')

executable(
  'minic-lsp',
  lfiles,
  pfiles,
  'symtab.c',
  dependencies : [ dependency('libcjson') ],
  install : true
)
