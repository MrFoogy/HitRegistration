@echo off
FOR /F "tokens=*" %%g IN ('.\date.bat') do (SET FOLDERNAME=D:\Exjobb-Jonathan\HitRegistration\FPSTemplate\DebugLogs\%%g)
mkdir %FOLDERNAME%

for %%s in (400 200 100 0) do (
	for %%t in ("True") do (
		for %%u in ("Straight") do (
			for %%v in ("True") do (
				call .\run_test.bat %%s %%t %%u Discrepancy %%v %FOLDERNAME%\\%%t-%%u-%%v-%%s
			)
		)
	)
)
