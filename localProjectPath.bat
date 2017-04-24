@if "%_USER_SPECIFIC%" == "" @(
	@set _USER_SPECIFIC=default
)
@if "%_PROJECT_PATH%" == "" @(
	@set _PROJECT_PATH=%~dp0
)