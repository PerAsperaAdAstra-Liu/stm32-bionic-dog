@echo off
rem 清理 Keil 编译产生的中间文件（工程交付/打包前运行一次）
cd /d %~dp0..
for /r %%i in (*.bak,*.dep,*.crf,*.o,*.d,*.lst,*.lnp,*.axf,*.htm,*.map,*.sct,*.build_log.htm,*.tra,*.iex) do del /q "%%i" 2>nul
if exist Project\OBJ rd /s /q Project\OBJ
if exist Project\LIST rd /s /q Project\LIST
echo 清理完成（Dog.hex 若已删除，重新编译一次即可生成）
pause
