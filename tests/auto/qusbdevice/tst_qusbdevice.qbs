CppApplication {
	Depends { name: 'Qt.core' }
	Depends { name: 'Qt.test' }
	Depends { name: 'QtUsb' }
	type: ['application', 'autotest']
	// type: base.concat(['autotest'])
	files: '*.cpp'
}
