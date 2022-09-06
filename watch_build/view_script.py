import gdspy
lib = gdspy.GdsLibrary(infile='library.gds')
tops = lib.top_level()
for cell in tops:
  filename = f'library.{cell.name}.svg'
  cell.write_svg(filename)
  print(f'wrote {filename}')

