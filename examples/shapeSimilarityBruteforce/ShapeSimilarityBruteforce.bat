@call ../../localProjectPath.bat
@call "%_PROJECT_PATH%/globalVariables.bat"

@set ARGUMENTS=Registration "%_PROJECT_PATH%/data/dataset1" "configShapeSimilarityBruteforce.txt" "generated" -debugImages
%_TACE% %ARGUMENTS%

@if "%1" neq "-nopause" (
	@pause
)