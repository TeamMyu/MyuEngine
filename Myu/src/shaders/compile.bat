cd %~dp0
for %%f in (*.frag *.vert) do (
echo glslc %%f -o %%f .spv
glslc %%f -o ..\..\..\bin\Editor\Debug\shaders\%%f.spv
)