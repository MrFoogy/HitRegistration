@echo off
FOR /F "tokens=*" %%g IN ('.\date.bat') do (SET FOLDERNAME=D:\Exjobb-Jonathan\HitRegistration\FPSTemplate\DebugLogs\%%g)
mkdir %FOLDERNAME%

for %%s in (0 200 100 400) do (
	for %%t in ("True" "False") do (
		for %%u in ("Alternate" "Straight") do (
			for %%v in ("True" "False") do (
				call .\run_test.bat %%s %%t %%u Discrepancy %%v %FOLDERNAME%\\%%t-%%u-%%v-%%s
			)
		)
	)
)
