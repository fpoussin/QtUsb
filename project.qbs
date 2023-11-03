import qbs.FileInfo
import qbs.Probes

Project {
	property string installContentsPath
	property string installLibraryDir

	property bool withExternalDeps: true

	qbsSearchPaths: ['qbs']

	references: [
		'examples/examples.qbs',
		'src/src.qbs',
		'tests/tests.qbs'
	]

	SubProject {
		condition: conan.found
		filePath: FileInfo.joinPaths(conan.generatedFilesPath, 'conanbuildinfo.qbs')
	}

	Probes.ConanfileProbe {
		id: conan
		condition: withExternalDeps
		conanfilePath: FileInfo.joinPaths(path, 'conanfile.txt')
		additionalArguments: ['--build=missing']

		settings: ({
			arch: qbs.architecture === 'arm64'? 'armv8' : 'x86_64',
			build_type: qbs.configurationName === 'release'? 'Release' : 'Debug',
		})
	}
}
