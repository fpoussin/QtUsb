CppApplication {
	Depends { name: 'Qt.core' }
	Depends { name: 'Qt.test' }
	Depends { name: 'QtUsb' }
	type: base.concat(['autotest'])
	files: '*.cpp'
}
