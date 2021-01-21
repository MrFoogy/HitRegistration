 path to rem // RunUAT.bat
 set UAT_PATH="D:\Exjobb-Jonathan\HitRegistration\UE\Engine\Build\BatchFiles\RunUAT.bat"
 rem // project name 
 set PRJ_NAME=D:\Exjobb-Jonathan\HitRegistration\FPSTemplate\FPSTemplate.uproject
 rem // Staging directory path
 set STAGING_DIR=D:\Exjobb-Jonathan\HitRegistration\FPSTemplate\Saved\StagedBuilds
 rem // test execution command
 set TEST_CMD=BuildCookRun
 rem // test name
 set TEST_NAME=HitRegTest
 rem // platform
 set PLATFORM=Win64
 rem // build configuration
 set CONFIG=Development
 
 rem ********************* Start Gauntlet Test *********************
 %UAT_PATH% %TEST_CMD% -project=%PRJ_NAME% -platform=%PLATFORM% -configuration=%CONFIG% -test=%TEST_NAME% -build -cook -pak -stage -stagingdirectory=%STAGING_DIR% 
 rem ********************* End   Gauntlet Test *********************
 pause
