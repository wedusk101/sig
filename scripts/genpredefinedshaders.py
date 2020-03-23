# (c) 2016-2019 Marcelo Kallmann
# Convert shader files to static string definitions

import os
import sys

shfolder = "../shaders/"
outfile = "gl_predef_shaders.inc"

numargs = len(sys.argv)

if numargs==2:
	shfolder = sys.argv[1]
elif numargs>2:
	shfolder = sys.argv[1]
	outfile = sys.argv[2]

files = os.listdir(shfolder) # get names before creating new files

out = open(outfile, 'w')
header = ( "//\n"
           "// Generated with SIG genpredefinedshaders.py - (c) Marcelo Kallmann 2016-2020\n"
           "//\n\n" )
out.writelines ( header )

for filename in files:
	shname = filename
	if shname.find(".txt")>=0 or shname.find(".bat")>=0 or shname.find(".inc")>=0 or shname.find("notused")>=0:
		print ( shname + " - skipped" )
		continue
	print ( shname + "..." )

	inp = open ( shfolder+filename )

	shname = shname.replace('.','_')
	out.writelines ( "static const char* pds_" + shname + "=\n" )

	for line in inp:
		l = len(line);
		if l>1 : # a \n counts as 1
			nb = line.find("//")
			if nb>=0 :
				line = line [0:nb]
			else:
				line = line [0:l-1]
			line = line.strip()
			line = line.replace('  ',' ')
			line = line.replace(' =','=')
			line = line.replace('= ','=')
			line = line.replace(' *','*')
			line = line.replace('* ','*')
			line = line.replace(' /','/')
			line = line.replace('/ ','/')
			line = line.replace(' (','(')
			line = line.replace('( ','(')
			line = line.replace(' )',')')
			line = line.replace(') ',')')
			line = line.replace(', ',',')
			l = len(line);
			if l>0 :
				if line[l-1]!=';' and line[l-1]!='{' and line[l-1]!='}' and line[l-1]!=')':
					line = line + "\\n"
				out.writelines ( "\"" + line + "\"\n" )

	out.writelines(";\n")
	inp.close()

out.close()
print ( "done." )
