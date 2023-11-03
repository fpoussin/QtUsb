Project {
	references: [
		'auto/qhiddevice/tst_qhiddevice.qbs',
		'auto/qusb/tst_qusb.qbs',
		'auto/qusbdevice/tst_qusbdevice.qbs',
		'auto/qusbendpoint/tst_qusbendpoint.qbs',
	]

	AutotestRunner {
		Depends { name: 'Qt.core'; versionAtLeast: '6.4' }
		Depends { name: 'QtUsb' }

		environment: {
			var env = base

			// Windows
			if (qbs.hostOS.contains('windows') && qbs.targetOS.contains('windows')) {
				var withoutQt = 'PATH='
				var withQt = 'PATH=' + Qt.core.binPath + ';'

				var pathIndex = -1
				for (var i = 0; i < env.length; i++) {
					if (env[i].startsWith(withoutQt)) {
						pathIndex = i
						break
					}
				}

				if (pathIndex != -1)
					env[pathIndex] = env[pathIndex].replace(withoutQt, withQt)
				else
					env.push(withQt)
			}

			// Mac
			if (qbs.hostOS.contains('darwin') && qbs.targetOS.contains('darwin')) {
				env.push('DYLD_FRAMEWORK_PATH=' + cpp.frameworkPaths + ':' + Qt.core.libPath)
				env.push('DYLD_LIBRARY_PATH=' + cpp.libraryPaths + ':' + Qt.core.libPath)
			}

			return env
		}
	}
}
