@echo off
FOR /F "tokens=*" %%g IN ('.\date.bat') do (SET FOLDERNAME=D:\Exjobb-Jonathan\HitRegistration\FPSTemplate\DebugLogs\%%g)
mkdir %FOLDERNAME%

for %%s in (0 100 200 400) do (
	for %%t in ("True") do (
		for %%u in ("Straight") do (
			for %%v in ("True") do (
				call .\run_test.bat %%s %%t %%u Fudge %%v %FOLDERNAME%\\%%t-%%u-%%v-%%s
			)
		)
	)
)
