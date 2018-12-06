import os

def get_project_conf(flags, log):
	lib_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'lib')
	return flags + ['-I', lib_path, '-I', '/usr/include/glib-2.0', '-I', '/usr/lib/x86_64-linux-gnu/glib-2.0/include']
