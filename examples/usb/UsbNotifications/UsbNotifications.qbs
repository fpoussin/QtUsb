CppApplication {
	Depends { name: 'Qt'; submodules: ['core', 'gui']}
	Depends { name: 'QtUsb' }

	files: ['*.h', '*.cpp']
}
