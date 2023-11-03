import qbs.FileInfo
import qbs.Probes

Project {
	references: [
		'examples/examples.qbs',
		'src/src.qbs',
		'tests/tests.qbs',
		FileInfo.joinPaths(conan.generatedFilesPath, 'conanbuildinfo.qbs'),
	]

	Probes.ConanfileProbe {
		id: conan
		conanfilePath: FileInfo.joinPaths(project.sourceDirectory, 'conanfile.txt')
		additionalArguments: ['--build=missing']

		settings: ({
			arch: qbs.architecture === 'arm64'? 'armv8' : 'x86_64',
			build_type: qbs.configurationName === 'release'? 'Release' : 'Debug',
		})
	}
}
