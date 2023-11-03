import qbs.FileInfo

Project {
	DynamicLibrary {
		Depends { name: 'cpp' }
		Depends { name: 'Qt.core' }
		Depends { name: 'Qt.core-private' }
		Depends { name: 'texttemplate' }

		Depends { name: 'hidapi' }
		Depends { name: 'libusb' }

		name: 'QtUsb'

		cpp.includePaths: [
			'include',
			'include/QtUsb',
		]

		files: ['usb/**']  /**/

		Group {
			name: 'Public headers'
			prefix: 'include/QtUsb/'
			files: '**'
		}

		Export {
			Depends { name: 'cpp' }
			cpp.systemIncludePaths: [
				FileInfo.joinPaths(exportingProduct.sourceDirectory, 'include'),
				FileInfo.joinPaths(exportingProduct.sourceDirectory, 'include/QtUsb'),
			]
			cpp.frameworkPaths: [exportingProduct.buildDirectory]
			cpp.libraryPaths: [exportingProduct.buildDirectory]
		}

		Group {
			fileTagsFilter: 'bundle.content'
			qbs.install: true
			qbs.installSourceBase: product.buildDirectory
			qbs.installPrefix: project.installContentsPath
			qbs.installDir: project.installLibraryDir
		}
	}
}
