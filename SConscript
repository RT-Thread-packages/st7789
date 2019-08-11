from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add st7789 src files.
if GetDepend('PKG_USING_ST7789'):
    src += Glob('src/st7789.c')

# add st7789 include path.
path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('st7789', src, depend = ['PKG_USING_ST7789'], CPPPATH = path)

Return('group')
