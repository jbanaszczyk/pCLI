
# Google Test & VS 2017

Directory contains gmock.sln prepared for Visual Studio.

Add the files to the googletest submodule repository, open solution file and (batch) build all targets/configurations.

# Enhancements:

* Minor fix in inclusions for Release config
* Added x64 support
* Modified OutDir. Its structure simplifies importing. Use: ```<AdditionalLibraryDirectories>$(SolutionDir)..\Libraries\googletest\googlemock\msvc\$(PlatFormToolsetVersion)\$(Platform)\$(Configuration);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>``` for all targets/configurations
